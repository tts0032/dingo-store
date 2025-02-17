// Copyright (c) 2023 dingodb.com, Inc. All Rights Reserved
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef DINGODB_SDK_VECTOR_H_
#define DINGODB_SDK_VECTOR_H_

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

#include "sdk/status.h"

namespace dingodb {
namespace sdk {

enum VectorIndexType : uint8_t { kNoneIndexType, kFlat, kIvfFlat, kIvfPq, kHnsw, kDiskAnn, kBruteForce };

enum MetricType : uint8_t { kNoneMetricType, kL2, kInnerProduct, kCosine };

struct FlatParam {
  explicit FlatParam(int32_t p_dimension, MetricType p_metric_type)
      : dimension(p_dimension), metric_type(p_metric_type) {}

  static VectorIndexType Type() { return VectorIndexType::kFlat; }

  // dimensions required
  int32_t dimension;
  // distance calculation method (L2 or InnerProduct) required
  MetricType metric_type;
};

struct IvfFlatParam {
  explicit IvfFlatParam(int32_t p_dimension, MetricType p_metric_type)
      : dimension(p_dimension), metric_type(p_metric_type) {}

  static VectorIndexType Type() { return VectorIndexType::kIvfFlat; }

  // dimensions required
  int32_t dimension;
  // distance calculation method (L2 or InnerProduct) required
  MetricType metric_type;
  // Number of cluster centers Default 2048 required
  int32_t ncentroids{2048};
};

struct IvfPqParam {
  explicit IvfPqParam(int32_t p_dimension, MetricType p_metric_type)
      : dimension(p_dimension), metric_type(p_metric_type) {}

  static VectorIndexType Type() { return VectorIndexType::kIvfPq; }

  // dimensions required
  int32_t dimension;
  // distance calculation method (L2 or InnerProduct) required
  MetricType metric_type;
  // Number of cluster centers Default 2048 required
  int32_t ncentroids{2048};
  // PQ split sub-vector size default 64 required
  int32_t nsubvector{64};
  // Inverted list (IVF) bucket initialization size default 1000 optional
  int32_t bucket_init_size{1000};
  // Inverted list (IVF) bucket maximum capacity default 1280000 optional
  int32_t bucket_max_size{1280000};
  // bit number of sub cluster center. default 8 required.  means 256.
  int32_t nbits_per_idx{8};
};

struct HnswParam {
  explicit HnswParam(int32_t p_dimension, MetricType p_metric_type, int32_t p_max_elements)
      : dimension(p_dimension), metric_type(p_metric_type), max_elements(p_max_elements) {}

  static VectorIndexType Type() { return VectorIndexType::kHnsw; }

  // dimensions required
  int32_t dimension;
  // distance calculation method (L2 or InnerProduct) required
  MetricType metric_type;
  // the range traversed in the graph during the process of finding node neighbors when
  // composing the graph. The larger the value, the better the composition effect and the
  // longer the composition time. Default 40 required
  int32_t ef_construction{40};
  // Set the maximum number of elements. required
  int32_t max_elements = 4;
  // The number of node neighbors, the larger the value, the better the composition effect, and the
  // more memory it takes. Default 32. required .
  int32_t nlinks{32};
};

struct DiskAnnParam {
  // TODO: to support
};

struct BruteForceParam {
  explicit BruteForceParam(int32_t p_dimension, MetricType p_metric_type)
      : dimension(p_dimension), metric_type(p_metric_type) {}

  static VectorIndexType Type() { return VectorIndexType::kBruteForce; }

  // dimensions required
  int32_t dimension;
  // distance calculation method (L2 or InnerProduct) required
  MetricType metric_type;
};

enum ValueType : uint8_t { kFloat, kUinT8 };

struct Vector {
  int32_t dimension;
  ValueType value_type;
  std::vector<float> float_values;
  std::vector<std::string> binary_values;

  Vector(Vector&& other) noexcept
      : dimension(other.dimension),
        value_type(other.value_type),
        float_values(std::move(other.float_values)),
        binary_values(std::move(other.binary_values)) {}

  Vector& operator=(Vector&& other) noexcept {
    dimension = other.dimension;
    value_type = other.value_type;
    float_values = std::move(other.float_values);
    binary_values = std::move(other.binary_values);
    return *this;
  }

  Vector(const Vector&) = delete;
  const Vector& operator=(const Vector&) = delete;
};

struct VectorWithId {
  int64_t id;
  Vector vector;
  //  TODO: scalar data and table data

  VectorWithId(VectorWithId&& other) noexcept : id(other.id), vector(std::move(other.vector)) {}

  VectorWithId& operator=(VectorWithId&& other) noexcept {
    id = other.id;
    vector = std::move(other.vector);
    return *this;
  }

  VectorWithId(const VectorWithId&) = delete;
  const VectorWithId& operator=(const VectorWithId&) = delete;
};

enum FilterSource : uint8_t {
  kNoneFilterSource,
  // filter vector scalar include post filter and pre filter
  kScalarFilter,
  // use coprocessor only include pre filter
  kTableFilter,
  // vector id search direct by ids. only include pre filter
  kVectorIdFilter
};

enum FilterType : uint8_t {
  kNoneFilterType,
  // first vector search, then filter
  kQueryPost,
  // first search from rocksdb, then search vector
  kQueryPre
};

enum SearchExtraParamType : uint8_t { kParallelOnQueries, kNprobe, kRecallNum, kEfSearch };

struct SearchParameter {
  SearchParameter(SearchParameter&& other) noexcept
      : topk(other.topk),
        with_vector_data(other.with_vector_data),
        with_scalar_data(other.with_scalar_data),
        with_table_data(other.with_table_data),
        enable_range_search(other.enable_range_search),
        radius(other.radius),
        filter_source(other.filter_source),
        filter_type(other.filter_type),
        vector_ids(std::move(other.vector_ids)),
        use_brute_force(other.use_brute_force),
        extra_params(std::move(other.extra_params)) {
    other.topk = 0;
    other.with_vector_data = true;
    other.with_scalar_data = false;
    other.with_table_data = false;
    other.enable_range_search = false;
    other.radius = 0.0f;
    other.filter_source = kNoneFilterSource;
    other.filter_type = kNoneFilterType;
    other.use_brute_force = false;
  }

  SearchParameter& operator=(SearchParameter&& other) noexcept {
    topk = other.topk;
    with_vector_data = other.with_vector_data;
    with_scalar_data = other.with_scalar_data;
    with_table_data = other.with_table_data;
    enable_range_search = other.enable_range_search;
    radius = other.radius;
    filter_source = other.filter_source;
    filter_type = other.filter_type;
    vector_ids = std::move(other.vector_ids);
    use_brute_force = other.use_brute_force;
    extra_params = std::move(other.extra_params);

    other.topk = 0;
    other.with_vector_data = true;
    other.with_scalar_data = false;
    other.with_table_data = false;
    other.enable_range_search = false;
    other.radius = 0.0f;
    other.filter_source = kNoneFilterSource;
    other.filter_type = kNoneFilterType;
    other.use_brute_force = false;

    return *this;
  }

  int32_t topk{0};
  bool with_vector_data{true};
  bool with_scalar_data{false};
  //  TODO: selected keys
  bool with_table_data{false};      // Default false, if true, response without table data
  bool enable_range_search{false};  // if enable_range_search = true. top_n disabled.
  float radius{0.0f};
  FilterSource filter_source{kNoneFilterSource};
  FilterType filter_type{kNoneFilterType};
  // TODO: coprocessorv2
  std::vector<int64_t> vector_ids;  // vector id array vector_filter == VECTOR_ID_FILTER enable vector_ids
  bool use_brute_force{false};      // use brute-force search
  std::unordered_map<SearchExtraParamType, int32_t> extra_params;  // The search method to use
};

struct SearchResult {
  VectorWithId vector_data;
  float distance;
  MetricType metric_type;

  SearchResult(SearchResult&& other) noexcept
      : vector_data(std::move(other.vector_data)), distance(other.distance), metric_type(other.metric_type) {}

  SearchResult& operator=(SearchResult&& other) noexcept {
    vector_data = std::move(other.vector_data);
    distance = other.distance;
    metric_type = other.metric_type;
    return *this;
  }

  SearchResult(const SearchResult&) = delete;
  const SearchResult& operator=(const SearchResult&) = delete;
};

struct DeleteResult {
  int64_t vector_id;
  bool deleted;
};

class VectorIndexCreator {
 public:
  ~VectorIndexCreator();

  VectorIndexCreator& SetSchemaId(int64_t schema_id);

  VectorIndexCreator& SetName(const std::string& name);

  VectorIndexCreator& SetRangePartitions(std::vector<int64_t> separator_id);

  VectorIndexCreator& SetReplicaNum(int64_t num);

  // one of FlatParam/IvfFlatParam/HnswParam/DiskAnnParam/BruteForceParam, if set multiple, the last one will effective
  VectorIndexCreator& SetFlatParam(const FlatParam& params);
  VectorIndexCreator& SetIvfFlatParam(const IvfFlatParam& params);
  VectorIndexCreator& SetIvfPqParam(const IvfPqParam& params);
  VectorIndexCreator& SetHnswParam(const HnswParam& params);
  // VectorIndexCreator& SetDiskAnnParam(DiskAnnParam& params);
  VectorIndexCreator& SetBruteForceParam(const BruteForceParam& params);

  // VectorIndexCreator& SetAutoIncrement(bool auto_incr);

  // VectorIndexCreator& SetAutoIncrementStart(int64_t start_id);

  Status Create(int64_t& out_index_id);

 private:
  friend class Client;

  // own
  class Data;
  std::unique_ptr<Data> data_;
  explicit VectorIndexCreator(Data* data);
};

class VectorClient : public std::enable_shared_from_this<VectorClient> {
 public:
  VectorClient(const VectorClient&) = delete;
  const VectorClient& operator=(const VectorClient&) = delete;

  ~VectorClient();

  Status Add(int64_t index_id, const std::vector<VectorWithId>& vectors, bool replace_deleted = false,
             bool is_update = false);
  Status Add(const std::string& index_name, const std::vector<VectorWithId>& vectors, bool replace_deleted = false,
             bool is_update = false);

  Status Search(int64_t index_id, const SearchParameter& search_param, const std::vector<VectorWithId>& target_vectors,
                std::vector<SearchResult>& out_result);
  Status Search(const std::string& index_name, const SearchParameter& search_param,
                const std::vector<VectorWithId>& target_vectors, std::vector<SearchResult>& out_result);

  Status Delete(int64_t index_id, const std::vector<int64_t>& vector_ids, std::vector<DeleteResult>& out_result);
  Status Delete(const std::string& index_name, const std::vector<int64_t>& vector_ids,
                std::vector<DeleteResult>& out_result);

 private:
  friend class Client;

  // own
  class Data;
  std::unique_ptr<Data> data_;

  explicit VectorClient(Data* data);
};
}  // namespace sdk

}  // namespace dingodb
#endif  // DINGODB_SDK_VECTOR_H_