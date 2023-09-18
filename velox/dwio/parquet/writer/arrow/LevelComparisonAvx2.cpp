/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Adapted from Apache Arrow.

#define PARQUET_IMPL_NAMESPACE avx2
#include "velox/dwio/parquet/writer/arrow/LevelComparisonInc.h"
#undef PARQUET_IMPL_NAMESPACE

namespace facebook::velox::parquet::arrow::internal {

uint64_t
GreaterThanBitmapAvx2(const int16_t* levels, int64_t num_levels, int16_t rhs) {
  return avx2::GreaterThanBitmapImpl(levels, num_levels, rhs);
}

MinMax FindMinMaxAvx2(const int16_t* levels, int64_t num_levels) {
  return avx2::FindMinMaxImpl(levels, num_levels);
}

} // namespace facebook::velox::parquet::arrow::internal
