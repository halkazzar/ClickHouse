#pragma once

#include <Core/Block.h>
#include <Core/SortDescription.h>


namespace DB
{

/// Sort one block by `description`. If limit != 0, then the partial sort of the first `limit` rows is produced.
void sortBlock(Block & block, const SortDescription & description, size_t limit = 0);


/** Used only in StorageMergeTree to sort the data with INSERT.
  * Sorting is stable. This is important for keeping the order of rows in the CollapsingMergeTree engine
  *  - because based on the order of rows it is determined whether to delete or leave groups of rows when collapsing.
  * Collations are not supported. Partial sorting is not supported.
  */
void stableSortBlock(Block & block, const SortDescription & description);

/** Same as stableSortBlock, but do not sort the block, but only calculate the permutation of the values,
  *  so that you can rearrange the column values yourself.
  */
void stableGetPermutation(const Block & block, const SortDescription & description, IColumn::Permutation & out_permutation);


/** Quickly check whether the block is already sorted. If the block is not sorted - returns false as fast as possible.
  * Collations are not supported.
  */
bool isAlreadySorted(const Block & block, const SortDescription & description);

using ColumnsWithSortDescriptions = std::vector<std::pair<const IColumn *, SortColumnDescription>>;

struct PartialSortingLess
{
    const ColumnsWithSortDescriptions & columns;

    explicit PartialSortingLess(const ColumnsWithSortDescriptions & columns_) : columns(columns_) {}

    bool operator() (size_t a, size_t b) const
    {
        for (ColumnsWithSortDescriptions::const_iterator it = columns.begin(); it != columns.end(); ++it)
        {
            int res = it->second.direction * it->first->compareAt(a, b, *it->first, it->second.nulls_direction);
            if (res < 0)
                return true;
            else if (res > 0)
                return false;
        }
        return false;
    }
};

ColumnsWithSortDescriptions getColumnsWithSortDescription(const Block & block, const SortDescription & description);

}
