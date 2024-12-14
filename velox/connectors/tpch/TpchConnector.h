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
#pragma once

#include "velox/common/config/Config.h"
#include "velox/connectors/Connector.h"
#include "velox/connectors/tpch/TpchConnectorSplit.h"
#include "velox/tpch/gen/TpchGen.h"

namespace facebook::velox::connector::tpch {

class TpchConnector;

// TPC-H column handle only needs the column name (all columns are generated in
// the same way).
class TpchColumnHandle : public ColumnHandle {
 public:
  explicit TpchColumnHandle(const std::string& name) : name_(name) {}

  const std::string& name() const {
    return name_;
  }

 private:
  const std::string name_;
};

// TPC-H table handle uses the underlying enum to describe the target table.
class TpchTableHandle : public ConnectorTableHandle {
 public:
  explicit TpchTableHandle(
      std::string connectorId,
      velox::tpch::Table table,
      double scaleFactor = 1.0)
      : ConnectorTableHandle(std::move(connectorId)),
        table_(table),
        scaleFactor_(scaleFactor) {
    VELOX_CHECK_GE(scaleFactor, 0, "Tpch scale factor must be non-negative");
  }

  ~TpchTableHandle() override {}

  std::string toString() const override;

  velox::tpch::Table getTable() const {
    return table_;
  }

  double getScaleFactor() const {
    return scaleFactor_;
  }

 private:
  const velox::tpch::Table table_;
  double scaleFactor_;
};

class TpchDataSource : public DataSource {
 public:
  TpchDataSource(
      const std::shared_ptr<const RowType>& outputType,
      const std::shared_ptr<connector::ConnectorTableHandle>& tableHandle,
      const std::unordered_map<
          std::string,
          std::shared_ptr<connector::ColumnHandle>>& columnHandles,
      velox::memory::MemoryPool* pool);

  void addSplit(std::shared_ptr<ConnectorSplit> split) override;

  void addDynamicFilter(
      column_index_t /*outputChannel*/,
      const std::shared_ptr<common::Filter>& /*filter*/) override {
    VELOX_NYI("Dynamic filters not supported by TpchConnector.");
  }

  std::optional<RowVectorPtr> next(uint64_t size, velox::ContinueFuture& future)
      override;

  uint64_t getCompletedRows() override {
    return completedRows_;
  }

  uint64_t getCompletedBytes() override {
    return completedBytes_;
  }

  std::unordered_map<std::string, RuntimeCounter> runtimeStats() override {
    // TODO: Which stats do we want to expose here?
    return {};
  }

 private:
  RowVectorPtr projectOutputColumns(RowVectorPtr vector);

  velox::tpch::Table tpchTable_;
  double scaleFactor_{1.0};
  size_t tpchTableRowCount_{0};
  RowTypePtr outputType_;

  // Mapping between output columns and their indices (column_index_t) in the
  // dbgen generated datasets.
  std::vector<column_index_t> outputColumnMappings_;

  std::shared_ptr<TpchConnectorSplit> currentSplit_;

  // First (splitOffset_) and last (splitEnd_) row number that should be
  // generated by this split.
  uint64_t splitOffset_{0};
  uint64_t splitEnd_{0};

  size_t completedRows_{0};
  size_t completedBytes_{0};

  memory::MemoryPool* pool_;
};

class TpchConnector final : public Connector {
 public:
  TpchConnector(
      const std::string& id,
      std::shared_ptr<const config::ConfigBase> config,
      folly::Executor* /*executor*/)
      : Connector(id) {}

  std::unique_ptr<DataSource> createDataSource(
      const std::shared_ptr<const RowType>& outputType,
      const std::shared_ptr<ConnectorTableHandle>& tableHandle,
      const std::unordered_map<
          std::string,
          std::shared_ptr<connector::ColumnHandle>>& columnHandles,
      ConnectorQueryCtx* connectorQueryCtx) override final {
    return std::make_unique<TpchDataSource>(
        outputType,
        tableHandle,
        columnHandles,
        connectorQueryCtx->memoryPool());
  }

  std::unique_ptr<DataSink> createDataSink(
      RowTypePtr /*inputType*/,
      std::shared_ptr<
          ConnectorInsertTableHandle> /*connectorInsertTableHandle*/,
      ConnectorQueryCtx* /*connectorQueryCtx*/,
      CommitStrategy /*commitStrategy*/) override final {
    VELOX_NYI("TpchConnector does not support data sink.");
  }
};

class TpchConnectorFactory : public ConnectorFactory {
 public:
  static constexpr const char* kTpchConnectorName{"tpch"};

  TpchConnectorFactory() : ConnectorFactory(kTpchConnectorName) {}

  explicit TpchConnectorFactory(const char* connectorName)
      : ConnectorFactory(connectorName) {}

  std::shared_ptr<Connector> newConnector(
      const std::string& id,
      std::shared_ptr<const config::ConfigBase> config,
      folly::Executor* ioExecutor = nullptr,
      folly::Executor* cpuExecutor = nullptr) override {
    return std::make_shared<TpchConnector>(id, config, ioExecutor);
  }
};

} // namespace facebook::velox::connector::tpch
