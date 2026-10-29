<a name="reference"></a>
<h1>
  <a href="../../resources/docs/reference/h1-reference.svg#gh-light-mode-only">
    <img src="../../resources/docs/reference/h1-reference.svg" alt="Reference">
  </a>
  <a href="../../resources/docs/reference/h1-reference-dark.svg#gh-dark-mode-only">
    <img src="../../resources/docs/reference/h1-reference-dark.svg" alt="Reference">
  </a>
</h1>

Looking for a precise signature or a default? Start with the header that owns it. The index lists supported public headers; implementation-only headers retain short status pages. For a walkthrough, see the [guide](../guide/README.md).

Implementation declarations live in *detail* namespaces and are not supported API.
**Compatibility:** the handler factories and signature adapters, engine dispatch
helpers, platform connection machinery, engine-factory helpers, TLS session
construction, and hashing/date/temp-file helpers have moved out of their former
public namespaces. Use route registration, public server/transport constructors,
and the documented customization contracts. No compatibility aliases retain the
old names; the processing behavior is unchanged.

<a name="common"></a>
<h2>
  <a href="../../resources/docs/reference/h2-common.svg#gh-light-mode-only">
    <img src="../../resources/docs/reference/h2-common.svg" alt="common">
  </a>
  <a href="../../resources/docs/reference/h2-common-dark.svg#gh-dark-mode-only">
    <img src="../../resources/docs/reference/h2-common-dark.svg" alt="common">
  </a>
</h2>

- [common/byte_storage.h](common/byte_storage.md)
- [common/cache.h](common/cache.md)
- [common/cache_policies.h](common/cache_policies.md)
- [common/cache_statistics.h](common/cache_statistics.md)
- [common/console_line_linux.h](common/console_line_linux.md)
- [common/console_line_windows.h](common/console_line_windows.md)
- [common/console_logger.h](common/console_logger.md)
- [common/console_logger_linux.h](common/console_logger_linux.md)
- [common/console_logger_windows.h](common/console_logger_windows.md)
- [common/date_server.h](common/date_server.md)
- [common/filesystem.h](common/filesystem.md)
- [common/filesystem_linux.h](common/filesystem_linux.md)
- [common/filesystem_windows.h](common/filesystem_windows.md)
- [common/hash_map.h](common/hash_map.md)
- [common/hash_set.h](common/hash_set.md)
- [common/reader.h](common/reader.md)
- [common/signaler.h](common/signaler.md)
- [common/signaler_linux.h](common/signaler_linux.md)
- [common/signaler_windows.h](common/signaler_windows.md)
- [common/task.h](common/task.md)
  - [task::promise_type](common/promise_type.md)
    - [task::promise_type::final_awaiter](common/final_awaiter.md)
- [common/task_awaiters.h](common/task_awaiters.md)
- [common/task_scheduler.h](common/task_scheduler.md)
- [common/writer.h](common/writer.md)

<a name="top-level"></a>
<h2>
  <a href="../../resources/docs/reference/h2-top-level.svg#gh-light-mode-only">
    <img src="../../resources/docs/reference/h2-top-level.svg" alt="top-level">
  </a>
  <a href="../../resources/docs/reference/h2-top-level-dark.svg#gh-dark-mode-only">
    <img src="../../resources/docs/reference/h2-top-level-dark.svg" alt="top-level">
  </a>
</h2>

- [contracts.h](contracts.md)
- [http_server.h](http_server.md)
- [https_server.h](https_server.md)
- [platform.h](platform.md)
- [server.h](server.md)
- [version.h](version.md)

<a name="network"></a>
<h2>
  <a href="../../resources/docs/reference/h2-network.svg#gh-light-mode-only">
    <img src="../../resources/docs/reference/h2-network.svg" alt="network">
  </a>
  <a href="../../resources/docs/reference/h2-network-dark.svg#gh-dark-mode-only">
    <img src="../../resources/docs/reference/h2-network-dark.svg" alt="network">
  </a>
</h2>


<a name="protocol"></a>
<h2>
  <a href="../../resources/docs/reference/h2-protocol.svg#gh-light-mode-only">
    <img src="../../resources/docs/reference/h2-protocol.svg" alt="protocol">
  </a>
  <a href="../../resources/docs/reference/h2-protocol-dark.svg#gh-dark-mode-only">
    <img src="../../resources/docs/reference/h2-protocol-dark.svg" alt="protocol">
  </a>
</h2>

- [protocol/contracts.h](protocol/contracts.md)
- [protocol/deserialization.h](protocol/deserialization.md)
- [protocol/http/common/auth/authentication_result.h](protocol/http/common/auth/authentication_result.md)
- [protocol/http/common/auth/basic.h](protocol/http/common/auth/basic.md)
- [protocol/http/common/auth/bearer.h](protocol/http/common/auth/bearer.md)
- [protocol/http/common/auth/contracts.h](protocol/http/common/auth/contracts.md)
- [protocol/http/common/header.h](protocol/http/common/header.md)
- [protocol/http/common/header_names.h](protocol/http/common/header_names.md)
- [protocol/http/common/headers/accept.h](protocol/http/common/headers/accept.md)
- [protocol/http/common/headers/accept_charset.h](protocol/http/common/headers/accept_charset.md)
- [protocol/http/common/headers/accept_encoding.h](protocol/http/common/headers/accept_encoding.md)
- [protocol/http/common/headers/accept_language.h](protocol/http/common/headers/accept_language.md)
- [protocol/http/common/headers/accept_ranges.h](protocol/http/common/headers/accept_ranges.md)
- [protocol/http/common/headers/access_control_allow_credentials.h](protocol/http/common/headers/access_control_allow_credentials.md)
- [protocol/http/common/headers/access_control_allow_headers.h](protocol/http/common/headers/access_control_allow_headers.md)
- [protocol/http/common/headers/access_control_allow_methods.h](protocol/http/common/headers/access_control_allow_methods.md)
- [protocol/http/common/headers/access_control_allow_origin.h](protocol/http/common/headers/access_control_allow_origin.md)
- [protocol/http/common/headers/access_control_expose_headers.h](protocol/http/common/headers/access_control_expose_headers.md)
- [protocol/http/common/headers/access_control_max_age.h](protocol/http/common/headers/access_control_max_age.md)
- [protocol/http/common/headers/access_control_request_headers.h](protocol/http/common/headers/access_control_request_headers.md)
- [protocol/http/common/headers/access_control_request_method.h](protocol/http/common/headers/access_control_request_method.md)
- [protocol/http/common/headers/age.h](protocol/http/common/headers/age.md)
- [protocol/http/common/headers/allow.h](protocol/http/common/headers/allow.md)
- [protocol/http/common/headers/authentication_info.h](protocol/http/common/headers/authentication_info.md)
- [protocol/http/common/headers/authorization.h](protocol/http/common/headers/authorization.md)
- [protocol/http/common/headers/cache_control.h](protocol/http/common/headers/cache_control.md)
- [protocol/http/common/headers/content_encoding.h](protocol/http/common/headers/content_encoding.md)
- [protocol/http/common/headers/content_language.h](protocol/http/common/headers/content_language.md)
- [protocol/http/common/headers/content_location.h](protocol/http/common/headers/content_location.md)
- [protocol/http/common/headers/content_range.h](protocol/http/common/headers/content_range.md)
- [protocol/http/common/headers/content_type.h](protocol/http/common/headers/content_type.md)
- [protocol/http/common/headers/cookie.h](protocol/http/common/headers/cookie.md)
- [protocol/http/common/headers/date.h](protocol/http/common/headers/date.md)
- [protocol/http/common/headers/etag.h](protocol/http/common/headers/etag.md)
- [protocol/http/common/headers/expires.h](protocol/http/common/headers/expires.md)
- [protocol/http/common/headers/from.h](protocol/http/common/headers/from.md)
- [protocol/http/common/headers/if_match.h](protocol/http/common/headers/if_match.md)
- [protocol/http/common/headers/if_modified_since.h](protocol/http/common/headers/if_modified_since.md)
- [protocol/http/common/headers/if_none_match.h](protocol/http/common/headers/if_none_match.md)
- [protocol/http/common/headers/if_range.h](protocol/http/common/headers/if_range.md)
- [protocol/http/common/headers/if_unmodified_since.h](protocol/http/common/headers/if_unmodified_since.md)
- [protocol/http/common/headers/keep_alive.h](protocol/http/common/headers/keep_alive.md)
- [protocol/http/common/headers/last_modified.h](protocol/http/common/headers/last_modified.md)
- [protocol/http/common/headers/location.h](protocol/http/common/headers/location.md)
- [protocol/http/common/headers/origin.h](protocol/http/common/headers/origin.md)
- [protocol/http/common/headers/pragma.h](protocol/http/common/headers/pragma.md)
- [protocol/http/common/headers/proxy_connection.h](protocol/http/common/headers/proxy_connection.md)
- [protocol/http/common/headers/range.h](protocol/http/common/headers/range.md)
- [protocol/http/common/headers/referer.h](protocol/http/common/headers/referer.md)
- [protocol/http/common/headers/retry_after.h](protocol/http/common/headers/retry_after.md)
- [protocol/http/common/headers/sec_websocket_accept.h](protocol/http/common/headers/sec_websocket_accept.md)
- [protocol/http/common/headers/sec_websocket_extensions.h](protocol/http/common/headers/sec_websocket_extensions.md)
- [protocol/http/common/headers/sec_websocket_key.h](protocol/http/common/headers/sec_websocket_key.md)
- [protocol/http/common/headers/sec_websocket_protocol.h](protocol/http/common/headers/sec_websocket_protocol.md)
- [protocol/http/common/headers/sec_websocket_version.h](protocol/http/common/headers/sec_websocket_version.md)
- [protocol/http/common/headers/server.h](protocol/http/common/headers/server.md)
- [protocol/http/common/headers/set_cookie.h](protocol/http/common/headers/set_cookie.md)
- [protocol/http/common/headers/user_agent.h](protocol/http/common/headers/user_agent.md)
- [protocol/http/common/headers/vary.h](protocol/http/common/headers/vary.md)
- [protocol/http/common/headers/www_authenticate.h](protocol/http/common/headers/www_authenticate.md)
- [protocol/http/common/helpers.h](protocol/http/common/helpers.md)
- [protocol/http/common/method_names.h](protocol/http/common/method_names.md)
- [protocol/http/common/query_parameter.h](protocol/http/common/query_parameter.md)
- [protocol/http/common/router.h](protocol/http/common/router.md)
- [protocol/http/common/router_match_result.h](protocol/http/common/router_match_result.md)
- [protocol/http/common/status_codes.h](protocol/http/common/status_codes.md)
- [protocol/http/common/target.h](protocol/http/common/target.md)
- [protocol/http/v11/body/framer_chunked.h](protocol/http/v11/body/framer_chunked.md)
- [protocol/http/v11/body/framer_error.h](protocol/http/v11/body/framer_error.md)
- [protocol/http/v11/body/framer_raw.h](protocol/http/v11/body/framer_raw.md)
- [protocol/http/v11/body/framer_state.h](protocol/http/v11/body/framer_state.md)
- [protocol/http/v11/body/reader.h](protocol/http/v11/body/reader.md)
- [protocol/http/v11/body/reader_chunked.h](protocol/http/v11/body/reader_chunked.md)
- [protocol/http/v11/body/reader_error.h](protocol/http/v11/body/reader_error.md)
- [protocol/http/v11/body/reader_raw.h](protocol/http/v11/body/reader_raw.md)
- [protocol/http/v11/body/reader_state.h](protocol/http/v11/body/reader_state.md)
- [protocol/http/v11/body/writer.h](protocol/http/v11/body/writer.md)
- [protocol/http/v11/body/writer_chunked.h](protocol/http/v11/body/writer_chunked.md)
- [protocol/http/v11/body/writer_error.h](protocol/http/v11/body/writer_error.md)
- [protocol/http/v11/body/writer_raw.h](protocol/http/v11/body/writer_raw.md)
- [protocol/http/v11/body/writer_state.h](protocol/http/v11/body/writer_state.md)
- [protocol/http/v11/connection.h](protocol/http/v11/connection.md)
- [protocol/http/v11/context.h](protocol/http/v11/context.md)
  - [context::policies](protocol/http/v11/context_policies.md)
- [protocol/http/v11/contracts.h](protocol/http/v11/contracts.md)
- [protocol/http/v11/decoder.h](protocol/http/v11/decoder.md)
- [protocol/http/v11/engine.h](protocol/http/v11/engine.md)
- [protocol/http/v11/headers/connection.h](protocol/http/v11/headers/connection.md)
- [protocol/http/v11/headers/content_length.h](protocol/http/v11/headers/content_length.md)
- [protocol/http/v11/headers/expect.h](protocol/http/v11/headers/expect.md)
- [protocol/http/v11/headers/forwarded.h](protocol/http/v11/headers/forwarded.md)
- [protocol/http/v11/headers/host.h](protocol/http/v11/headers/host.md)
- [protocol/http/v11/headers/max_forwards.h](protocol/http/v11/headers/max_forwards.md)
- [protocol/http/v11/headers/rules/directives.h](protocol/http/v11/headers/rules/directives.md)
- [protocol/http/v11/headers/rules/framing.h](protocol/http/v11/headers/rules/framing.md)
- [protocol/http/v11/headers/rules/policy.h](protocol/http/v11/headers/rules/policy.md)
- [protocol/http/v11/headers/rules/routing.h](protocol/http/v11/headers/rules/routing.md)
- [protocol/http/v11/headers/te.h](protocol/http/v11/headers/te.md)
- [protocol/http/v11/headers/trailer.h](protocol/http/v11/headers/trailer.md)
- [protocol/http/v11/headers/transfer_encoding.h](protocol/http/v11/headers/transfer_encoding.md)
- [protocol/http/v11/headers/upgrade.h](protocol/http/v11/headers/upgrade.md)
- [protocol/http/v11/headers/via.h](protocol/http/v11/headers/via.md)
- [protocol/http/v11/headers/x_forwarded_for.h](protocol/http/v11/headers/x_forwarded_for.md)
- [protocol/http/v11/headers/x_forwarded_host.h](protocol/http/v11/headers/x_forwarded_host.md)
- [protocol/http/v11/headers/x_forwarded_proto.h](protocol/http/v11/headers/x_forwarded_proto.md)
- [protocol/http/v11/parsed_types.h](protocol/http/v11/parsed_types.md)
- [protocol/http/v11/policies.h](protocol/http/v11/policies.md)
- [protocol/http/v11/reason_phrases.h](protocol/http/v11/reason_phrases.md)
- [protocol/http/v11/rejection_reason.h](protocol/http/v11/rejection_reason.md)
- [protocol/http/v11/request.h](protocol/http/v11/request.md)
- [protocol/http/v11/response.h](protocol/http/v11/response.md)
- [protocol/http/v11/results.h](protocol/http/v11/results.md)
- [protocol/http/v11/static_file_server.h](protocol/http/v11/static_file_server.md)
- [protocol/http/v11/status_lines.h](protocol/http/v11/status_lines.md)
- [protocol/http/v11/verdict.h](protocol/http/v11/verdict.md)

<a name="transport"></a>
<h2>
  <a href="../../resources/docs/reference/h2-transport.svg#gh-light-mode-only">
    <img src="../../resources/docs/reference/h2-transport.svg" alt="transport">
  </a>
  <a href="../../resources/docs/reference/h2-transport-dark.svg#gh-dark-mode-only">
    <img src="../../resources/docs/reference/h2-transport-dark.svg" alt="transport">
  </a>
</h2>

- [transport/server/contracts.h](transport/server/contracts.md)
- [transport/server/tcp.h](transport/server/tcp.md)
- [transport/server/tcp_policies.h](transport/server/tcp_policies.md)
- [transport/server/tls.h](transport/server/tls.md)
- [transport/server/tls_policies.h](transport/server/tls_policies.md)
