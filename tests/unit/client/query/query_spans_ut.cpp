#include <src/client/query/impl/query_spans.h>
#include <tests/common/fake_trace_provider.h>

#include <gtest/gtest.h>

using namespace NYdb;
using namespace NYdb::NQuery;
using namespace NYdb::NMetrics;
using namespace NYdb::NTests;

class QuerySpanTest : public ::testing::Test {
protected:
    void SetUp() override {
        Tracer = std::make_shared<TFakeTracer>();
    }

    std::shared_ptr<TFakeTracer> Tracer;
};

TEST_F(QuerySpanTest, SpanNameFormat) {
    TQuerySpan span(Tracer, "ExecuteQuery", "localhost:2135");
    span.End(EStatus::SUCCESS);

    ASSERT_EQ(Tracer->SpanCount(), 1u);
    EXPECT_EQ(Tracer->GetLastSpanRecord().Name, "ydb.ExecuteQuery");
}

TEST_F(QuerySpanTest, SpanKindIsClient) {
    TQuerySpan span(Tracer, "CreateSession", "localhost:2135");
    span.End(EStatus::SUCCESS);

    ASSERT_EQ(Tracer->SpanCount(), 1u);
    EXPECT_EQ(Tracer->GetLastSpanRecord().Kind, ESpanKind::CLIENT);
}

TEST_F(QuerySpanTest, DbSystemAttribute) {
    TQuerySpan span(Tracer, "ExecuteQuery", "localhost:2135");
    span.End(EStatus::SUCCESS);

    auto fakeSpan = Tracer->GetLastSpan();
    ASSERT_NE(fakeSpan, nullptr);
    EXPECT_EQ(fakeSpan->GetStringAttribute("db.system.name"), "ydb");
}

TEST_F(QuerySpanTest, ServerAddressAndPort) {
    TQuerySpan span(Tracer, "Commit", "ydb.server:2135");
    span.End(EStatus::SUCCESS);

    auto fakeSpan = Tracer->GetLastSpan();
    ASSERT_NE(fakeSpan, nullptr);
    EXPECT_EQ(fakeSpan->GetStringAttribute("server.address"), "ydb.server");
    EXPECT_EQ(fakeSpan->GetIntAttribute("server.port"), 2135);
}

TEST_F(QuerySpanTest, ServerAddressCustomPort) {
    TQuerySpan span(Tracer, "Rollback", "myhost:9090");
    span.End(EStatus::SUCCESS);

    auto fakeSpan = Tracer->GetLastSpan();
    ASSERT_NE(fakeSpan, nullptr);
    EXPECT_EQ(fakeSpan->GetStringAttribute("server.address"), "myhost");
    EXPECT_EQ(fakeSpan->GetIntAttribute("server.port"), 9090);
}

TEST_F(QuerySpanTest, ServerAddressNoPortDefaultsTo2135) {
    TQuerySpan span(Tracer, "ExecuteQuery", "myhost");
    span.End(EStatus::SUCCESS);

    auto fakeSpan = Tracer->GetLastSpan();
    ASSERT_NE(fakeSpan, nullptr);
    EXPECT_EQ(fakeSpan->GetStringAttribute("server.address"), "myhost");
    EXPECT_EQ(fakeSpan->GetIntAttribute("server.port"), 2135);
}

TEST_F(QuerySpanTest, IPv6EndpointParsing) {
    TQuerySpan span(Tracer, "ExecuteQuery", "[::1]:2136");
    span.End(EStatus::SUCCESS);

    auto fakeSpan = Tracer->GetLastSpan();
    ASSERT_NE(fakeSpan, nullptr);
    EXPECT_EQ(fakeSpan->GetStringAttribute("server.address"), "::1");
    EXPECT_EQ(fakeSpan->GetIntAttribute("server.port"), 2136);
}

TEST_F(QuerySpanTest, IPv6EndpointNoPort) {
    TQuerySpan span(Tracer, "ExecuteQuery", "[fe80::1]");
    span.End(EStatus::SUCCESS);

    auto fakeSpan = Tracer->GetLastSpan();
    ASSERT_NE(fakeSpan, nullptr);
    EXPECT_EQ(fakeSpan->GetStringAttribute("server.address"), "fe80::1");
    EXPECT_EQ(fakeSpan->GetIntAttribute("server.port"), 2135);
}

TEST_F(QuerySpanTest, PeerEndpointAttributes) {
    TQuerySpan span(Tracer, "ExecuteQuery", "discovery.ydb:2135");
    span.SetPeerEndpoint("10.0.0.1:2136");
    span.End(EStatus::SUCCESS);

    auto fakeSpan = Tracer->GetLastSpan();
    ASSERT_NE(fakeSpan, nullptr);
    EXPECT_EQ(fakeSpan->GetStringAttribute("network.peer.address"), "10.0.0.1");
    EXPECT_EQ(fakeSpan->GetIntAttribute("network.peer.port"), 2136);
}

TEST_F(QuerySpanTest, SuccessStatusRecorded) {
    TQuerySpan span(Tracer, "Commit", "localhost:2135");
    span.End(EStatus::SUCCESS);

    auto fakeSpan = Tracer->GetLastSpan();
    ASSERT_NE(fakeSpan, nullptr);
    EXPECT_TRUE(fakeSpan->HasIntAttribute("db.response.status_code"));
    EXPECT_EQ(fakeSpan->GetIntAttribute("db.response.status_code"), static_cast<int64_t>(EStatus::SUCCESS));
    EXPECT_FALSE(fakeSpan->HasStringAttribute("error.type"));
}

TEST_F(QuerySpanTest, ErrorStatusSetsErrorType) {
    TQuerySpan span(Tracer, "Rollback", "localhost:2135");
    span.End(EStatus::UNAVAILABLE);

    auto fakeSpan = Tracer->GetLastSpan();
    ASSERT_NE(fakeSpan, nullptr);
    EXPECT_EQ(fakeSpan->GetIntAttribute("db.response.status_code"), static_cast<int64_t>(EStatus::UNAVAILABLE));
    EXPECT_TRUE(fakeSpan->HasStringAttribute("error.type"));
    EXPECT_FALSE(fakeSpan->GetStringAttribute("error.type").empty());
}

TEST_F(QuerySpanTest, SpanIsEndedAfterEnd) {
    TQuerySpan span(Tracer, "ExecuteQuery", "localhost:2135");
    auto fakeSpan = Tracer->GetLastSpan();
    ASSERT_NE(fakeSpan, nullptr);

    EXPECT_FALSE(fakeSpan->IsEnded());
    span.End(EStatus::SUCCESS);
    EXPECT_TRUE(fakeSpan->IsEnded());
}

TEST_F(QuerySpanTest, NullTracerDoesNotCrash) {
    EXPECT_NO_THROW({
        TQuerySpan span(nullptr, "ExecuteQuery", "localhost:2135");
        span.SetPeerEndpoint("10.0.0.1:2136");
        span.AddEvent("retry", {{"attempt", "1"}});
        span.End(EStatus::SUCCESS);
    });
}

TEST_F(QuerySpanTest, DestructorEndsSpan) {
    auto fakeSpan = [&]() -> std::shared_ptr<TFakeSpan> {
        TQuerySpan span(Tracer, "CreateSession", "localhost:2135");
        return Tracer->GetLastSpan();
    }();

    ASSERT_NE(fakeSpan, nullptr);
    EXPECT_TRUE(fakeSpan->IsEnded());
}

TEST_F(QuerySpanTest, ExplicitEndThenDestructorDoesNotDoubleEnd) {
    auto fakeSpan = [&]() -> std::shared_ptr<TFakeSpan> {
        TQuerySpan span(Tracer, "Commit", "localhost:2135");
        span.End(EStatus::SUCCESS);
        return Tracer->GetLastSpan();
    }();

    ASSERT_NE(fakeSpan, nullptr);
    EXPECT_TRUE(fakeSpan->IsEnded());
}

TEST_F(QuerySpanTest, AddEventForwarded) {
    TQuerySpan span(Tracer, "ExecuteQuery", "localhost:2135");
    span.AddEvent("retry", {{"ydb.attempt", "2"}, {"error.type", "UNAVAILABLE"}});
    span.End(EStatus::SUCCESS);

    auto fakeSpan = Tracer->GetLastSpan();
    ASSERT_NE(fakeSpan, nullptr);
    auto events = fakeSpan->GetEvents();
    ASSERT_EQ(events.size(), 1u);
    EXPECT_EQ(events[0].Name, "retry");
    EXPECT_EQ(events[0].Attributes.at("ydb.attempt"), "2");
    EXPECT_EQ(events[0].Attributes.at("error.type"), "UNAVAILABLE");
}

TEST_F(QuerySpanTest, EmptyPeerEndpointIgnored) {
    TQuerySpan span(Tracer, "CreateSession", "localhost:2135");
    span.SetPeerEndpoint("");
    span.End(EStatus::SUCCESS);

    auto fakeSpan = Tracer->GetLastSpan();
    ASSERT_NE(fakeSpan, nullptr);
    EXPECT_FALSE(fakeSpan->HasStringAttribute("network.peer.address"));
    EXPECT_FALSE(fakeSpan->HasIntAttribute("network.peer.port"));
}

TEST_F(QuerySpanTest, AllFourOperationNames) {
    const std::vector<std::string> operations = {"CreateSession", "ExecuteQuery", "Commit", "Rollback"};

    for (const auto& op : operations) {
        TQuerySpan span(Tracer, op, "localhost:2135");
        span.End(EStatus::SUCCESS);
    }

    auto spans = Tracer->GetSpans();
    ASSERT_EQ(spans.size(), 4u);
    EXPECT_EQ(spans[0].Name, "ydb.CreateSession");
    EXPECT_EQ(spans[1].Name, "ydb.ExecuteQuery");
    EXPECT_EQ(spans[2].Name, "ydb.Commit");
    EXPECT_EQ(spans[3].Name, "ydb.Rollback");

    for (const auto& record : spans) {
        EXPECT_EQ(record.Kind, ESpanKind::CLIENT);
    }
}

TEST_F(QuerySpanTest, MultipleErrorStatuses) {
    std::vector<EStatus> errorStatuses = {
        EStatus::BAD_REQUEST,
        EStatus::UNAUTHORIZED,
        EStatus::INTERNAL_ERROR,
        EStatus::UNAVAILABLE,
        EStatus::OVERLOADED,
        EStatus::TIMEOUT,
        EStatus::NOT_FOUND,
        EStatus::CLIENT_INTERNAL_ERROR,
    };

    for (auto status : errorStatuses) {
        TQuerySpan span(Tracer, "ExecuteQuery", "localhost:2135");
        span.End(status);

        auto fakeSpan = Tracer->GetLastSpan();
        ASSERT_NE(fakeSpan, nullptr);
        EXPECT_TRUE(fakeSpan->HasStringAttribute("error.type"))
            << "error.type missing for status " << static_cast<int>(status);
        EXPECT_EQ(fakeSpan->GetIntAttribute("db.response.status_code"), static_cast<int64_t>(status));
    }
}

TEST_F(QuerySpanTest, EmptyEndpointDoesNotCrash) {
    EXPECT_NO_THROW({
        TQuerySpan span(Tracer, "ExecuteQuery", "");
        span.End(EStatus::SUCCESS);
    });
}
