# YDB C++ SDK — OpenTelemetry Tracing Demo

Демонстрация трассировки операций QueryService (CreateSession, ExecuteQuery, Commit, Rollback)
с визуализацией в **Grafana**, **Jaeger** и **Prometheus**.

## Архитектура

```
┌──────────────┐     OTLP/HTTP      ┌──────────────────┐
│  C++ demo    │ ──────────────────> │  OTel Collector   │
│  application │                    │  :4328 (HTTP)     │
└──────────────┘                    └────────┬──────────┘
                                        │          │
                              traces    │          │  metrics
                                        ▼          ▼
                                  ┌──────────┐  ┌────────────┐
                                  │  Jaeger   │  │ Prometheus  │
                                  │  :16686   │  │ :9090       │
                                  └─────┬─────┘  └──────┬──────┘
                                        │               │
                                        └───────┬───────┘
                                                ▼
                                          ┌──────────┐
                                          │ Grafana   │
                                          │ :3000     │
                                          └──────────┘
```

## Быстрый старт

### 1. Запустить инфраструктуру

```bash
cd examples/otel_tracing
docker compose up -d
```

Дождитесь готовности YDB:

```bash
docker compose logs ydb -f
# Ждите строку "Database started successfully"
```

### 2. Собрать SDK с OTel и тестами

Из корня репозитория:

```bash
mkdir -p build && cd build

cmake .. \
  -DYDB_SDK_TESTS=ON \
  -DYDB_SDK_ENABLE_OTEL_TRACE=ON \
  -DYDB_SDK_ENABLE_OTEL_METRICS=ON

cmake --build . --target otel_tracing_example -j$(nproc)
```

### 3. Запустить демо

```bash
./examples/otel_tracing/otel_tracing_example \
  --endpoint grpc://localhost:2136 \
  --database /local \
  --otlp http://localhost:4328 \
  --iterations 20
```

### 4. Открыть дашборды

| Сервис     | URL                          | Описание                        |
|-----------|------------------------------|---------------------------------|
| Grafana   | http://localhost:3000         | Дашборд "YDB QueryService"     |
| Jaeger    | http://localhost:16686        | Поиск трейсов по сервису        |
| Prometheus| http://localhost:9090         | Метрики `ydb_ydb_query_*`      |

**Grafana**: логин `admin` / пароль `admin`.

### 5. Что смотреть

#### В Grafana (дашборд "YDB QueryService"):
- **Request Rate by Operation** — RPS по операциям (ExecuteQuery, CreateSession, Commit, Rollback)
- **Error Rate by Operation** — частота ошибок
- **Latency p50/p95/p99** — распределение задержек
- **Error Ratio** — процент ошибок
- **Recent Traces** — таблица трейсов из Jaeger

#### В Jaeger UI:
- Выберите сервис `ydb-cpp-sdk-demo`
- Каждый спан содержит атрибуты:
  - `db.system.name` = `ydb`
  - `server.address`, `server.port`
  - `network.peer.address`, `network.peer.port`
  - `db.query.text` (для ExecuteQuery)
  - `db.response.status_code`, `error.type` (при ошибках)

#### В Prometheus:
- `ydb_ydb_query_requests_total` — счётчик запросов
- `ydb_ydb_query_errors_total` — счётчик ошибок
- `ydb_ydb_query_latency_ms_bucket` — гистограмма задержек

### 6. Остановить

```bash
cd examples/otel_tracing
docker compose down -v
```
