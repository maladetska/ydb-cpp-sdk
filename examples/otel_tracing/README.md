# YDB C++ SDK — OpenTelemetry Demo

Демонстрация трассировки и метрик операций QueryService и TableService
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
| Prometheus| http://localhost:9090         | Метрики `db_client_operation_*` |

**Grafana**: логин `admin` / пароль `admin`.

### 5. Что смотреть

#### В Grafana (дашборд "YDB QueryService"):
- **Request Rate by Operation** — RPS по операциям (ExecuteQuery, ExecuteDataQuery, CreateSession, Commit, Rollback)
- **Error Rate by Operation** — частота ошибок
- **Duration p50/p95/p99** — распределение длительности операций
- **Error Ratio** — процент ошибок
- **Recent Traces** — таблица трейсов из Jaeger

#### В Jaeger UI:
- Выберите сервис `ydb-cpp-sdk-demo`
- Трейсы от QueryService содержат спаны с атрибутами:
  - `db.system.name` = `other_sql`
  - `server.address`, `server.port`
  - `db.query.text` (для ExecuteQuery)
  - `db.response.status_code`, `error.type` (при ошибках)
- TableService пока генерирует только метрики (без спанов)

#### В Prometheus:
- `db_client_operation_requests_total` — счётчик запросов по операциям
- `db_client_operation_errors_total` — счётчик ошибок по операциям
- `db_client_operation_duration_seconds_bucket` — гистограмма длительности (OTel Semantic Conventions)

### 6. Остановить

```bash
cd examples/otel_tracing
docker compose down -v
```
