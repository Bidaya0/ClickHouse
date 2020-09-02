#include <Storages/MergeTree/TTLMergeSelector.h>
#include <Storages/MergeTree/MergeTreeData.h>
#include <Parsers/queryToString.h>

#include <algorithm>
#include <cmath>


namespace DB
{

const String & getPartitionIdForPart(const ITTLMergeSelector::Part & part_info)
{
    const MergeTreeData::DataPartPtr & part = *static_cast<const MergeTreeData::DataPartPtr *>(part_info.data);
    return part->info.partition_id;
}


IMergeSelector::PartsInPartition ITTLMergeSelector::select(
    const Partitions & partitions,
    const size_t max_total_size_to_merge)
{
    using Iterator = IMergeSelector::PartsInPartition::const_iterator;
    Iterator best_begin;
    ssize_t partition_to_merge_index = -1;
    time_t partition_to_merge_min_ttl = 0;

    for (size_t i = 0; i < partitions.size(); ++i)
    {
        const auto & mergeable_parts_in_partition = partitions[i];
        if (mergeable_parts_in_partition.empty())
            continue;

        const auto & partition_id = getPartitionIdForPart(mergeable_parts_in_partition.front());
        const auto & next_merge_time_for_partition = merge_due_times[partition_id];
        if (next_merge_time_for_partition > current_time)
            continue;

        for (Iterator part_it = mergeable_parts_in_partition.cbegin(); part_it != mergeable_parts_in_partition.cend(); ++part_it)
        {
            time_t ttl = getTTLForPart(*part_it);

            if (ttl && !isTTLAlreadySatisfied(*part_it) && (partition_to_merge_index == -1 || ttl < partition_to_merge_min_ttl))
            {
                partition_to_merge_min_ttl = ttl;
                partition_to_merge_index = i;
                best_begin = part_it;
            }
        }
    }

    if (partition_to_merge_index == -1 || partition_to_merge_min_ttl > current_time)
        return {};

    const auto & best_partition = partitions[partition_to_merge_index];
    Iterator best_end = best_begin + 1;
    size_t total_size = 0;

    while (true)
    {
        time_t ttl = getTTLForPart(*best_begin);

        if (!ttl || isTTLAlreadySatisfied(*best_begin) || ttl > current_time
            || (max_total_size_to_merge && total_size > max_total_size_to_merge))
        {
            ++best_begin;
            break;
        }

        total_size += best_begin->size;
        if (best_begin == best_partition.begin())
            break;

        --best_begin;
    }

    while (best_end != best_partition.end())
    {
        time_t ttl = getTTLForPart(*best_end);

        if (!ttl || isTTLAlreadySatisfied(*best_end) || ttl > current_time
            || (max_total_size_to_merge && total_size > max_total_size_to_merge))
            break;

        total_size += best_end->size;
        ++best_end;
    }

    const auto & best_partition_id = getPartitionIdForPart(best_partition.front());
    merge_due_times[best_partition_id] = current_time + merge_cooldown_time;

    return PartsInPartition(best_begin, best_end);
}

time_t TTLDeleteMergeSelector::getTTLForPart(const IMergeSelector::Part & part) const
{
    return only_drop_parts ? part.ttl_infos.part_max_ttl : part.ttl_infos.part_min_ttl;
}

time_t TTLRecompressMergeSelector::getTTLForPart(const IMergeSelector::Part & part) const
{
    return part.ttl_infos.getMinRecompressionTTL();
}

bool TTLRecompressMergeSelector::isTTLAlreadySatisfied(const IMergeSelector::Part & part) const
{
    if (recompression_ttls.empty())
        return false;

    auto ttl_description = selectTTLEntryForTTLInfos(recompression_ttls, part.ttl_infos.recompression_ttl, current_time, false);

    if (!ttl_description)
        return true;

    auto ast_to_str = [](ASTPtr query) -> String
    {
        if (!query)
            return "";
        return queryToString(query);
    };
    //LOG_DEBUG(&Poco::Logger::get("RECOMPRESS SELECTOR"), "PART CODEC: {}", ast_to_str(part.compression_codec_desc));
    //LOG_DEBUG(&Poco::Logger::get("RECOMPRESS SELECTOR"), "ENTRY CODEC: {}", ast_to_str(ttl_description->recompression_codec));

    return ast_to_str(ttl_description->recompression_codec) == ast_to_str(part.compression_codec_desc);
}

}
