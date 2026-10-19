<a id="status-lines-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../resources/docs/reference/protocol/http/v11/status_lines/index-dark.svg">
    <img src="../../../../../resources/docs/reference/protocol/http/v11/status_lines/index.svg" alt="Index for status_lines.h">
  </picture>
</h1>

**Header:** [include/protocol/http/v11/status_lines.h](../../../../../include/protocol/http/v11/status_lines.h)

Provides the http 1.1 status lines.

**protocol::http::v11**

- [STR_VALUE_RAW](#macro-str-value-raw) · *macro*
- [STR_VALUE](#macro-str-value) · *macro*
- [EAS](#macro-eas) · *macro*
- [SL](#macro-sl) · *macro*
- [status_lines](#type-status-lines) · *type*

**protocol::http::v11::status_lines**

- [k100](#constant-k100) · *constant*
- [k101](#constant-k101) · *constant*
- [k200](#constant-k200) · *constant*
- [k201](#constant-k201) · *constant*
- [k202](#constant-k202) · *constant*
- [k203](#constant-k203) · *constant*
- [k204](#constant-k204) · *constant*
- [k205](#constant-k205) · *constant*
- [k206](#constant-k206) · *constant*
- [k300](#constant-k300) · *constant*
- [k301](#constant-k301) · *constant*
- [k302](#constant-k302) · *constant*
- [k303](#constant-k303) · *constant*
- [k304](#constant-k304) · *constant*
- [k305](#constant-k305) · *constant*
- [k306](#constant-k306) · *constant*
- [k307](#constant-k307) · *constant*
- [k308](#constant-k308) · *constant*
- [k400](#constant-k400) · *constant*
- [k401](#constant-k401) · *constant*
- [k402](#constant-k402) · *constant*
- [k403](#constant-k403) · *constant*
- [k404](#constant-k404) · *constant*
- [k405](#constant-k405) · *constant*
- [k406](#constant-k406) · *constant*
- [k407](#constant-k407) · *constant*
- [k408](#constant-k408) · *constant*
- [k409](#constant-k409) · *constant*
- [k410](#constant-k410) · *constant*
- [k411](#constant-k411) · *constant*
- [k412](#constant-k412) · *constant*
- [k413](#constant-k413) · *constant*
- [k414](#constant-k414) · *constant*
- [k415](#constant-k415) · *constant*
- [k416](#constant-k416) · *constant*
- [k417](#constant-k417) · *constant*
- [k418](#constant-k418) · *constant*
- [k421](#constant-k421) · *constant*
- [k422](#constant-k422) · *constant*
- [k426](#constant-k426) · *constant*
- [k431](#constant-k431) · *constant*
- [k500](#constant-k500) · *constant*
- [k501](#constant-k501) · *constant*
- [k502](#constant-k502) · *constant*
- [k503](#constant-k503) · *constant*
- [k504](#constant-k504) · *constant*
- [k505](#constant-k505) · *constant*
- [k100Sz](#constant-k100sz) · *constant*
- [k101Sz](#constant-k101sz) · *constant*
- [k200Sz](#constant-k200sz) · *constant*
- [k201Sz](#constant-k201sz) · *constant*
- [k202Sz](#constant-k202sz) · *constant*
- [k203Sz](#constant-k203sz) · *constant*
- [k204Sz](#constant-k204sz) · *constant*
- [k205Sz](#constant-k205sz) · *constant*
- [k206Sz](#constant-k206sz) · *constant*
- [k300Sz](#constant-k300sz) · *constant*
- [k301Sz](#constant-k301sz) · *constant*
- [k302Sz](#constant-k302sz) · *constant*
- [k303Sz](#constant-k303sz) · *constant*
- [k304Sz](#constant-k304sz) · *constant*
- [k305Sz](#constant-k305sz) · *constant*
- [k306Sz](#constant-k306sz) · *constant*
- [k307Sz](#constant-k307sz) · *constant*
- [k308Sz](#constant-k308sz) · *constant*
- [k400Sz](#constant-k400sz) · *constant*
- [k401Sz](#constant-k401sz) · *constant*
- [k402Sz](#constant-k402sz) · *constant*
- [k403Sz](#constant-k403sz) · *constant*
- [k404Sz](#constant-k404sz) · *constant*
- [k405Sz](#constant-k405sz) · *constant*
- [k406Sz](#constant-k406sz) · *constant*
- [k407Sz](#constant-k407sz) · *constant*
- [k408Sz](#constant-k408sz) · *constant*
- [k409Sz](#constant-k409sz) · *constant*
- [k410Sz](#constant-k410sz) · *constant*
- [k411Sz](#constant-k411sz) · *constant*
- [k412Sz](#constant-k412sz) · *constant*
- [k413Sz](#constant-k413sz) · *constant*
- [k414Sz](#constant-k414sz) · *constant*
- [k415Sz](#constant-k415sz) · *constant*
- [k416Sz](#constant-k416sz) · *constant*
- [k417Sz](#constant-k417sz) · *constant*
- [k418Sz](#constant-k418sz) · *constant*
- [k421Sz](#constant-k421sz) · *constant*
- [k422Sz](#constant-k422sz) · *constant*
- [k426Sz](#constant-k426sz) · *constant*
- [k431Sz](#constant-k431sz) · *constant*
- [k500Sz](#constant-k500sz) · *constant*
- [k501Sz](#constant-k501sz) · *constant*
- [k502Sz](#constant-k502sz) · *constant*
- [k503Sz](#constant-k503sz) · *constant*
- [k504Sz](#constant-k504sz) · *constant*
- [k505Sz](#constant-k505sz) · *constant*

<a id="macro-str-value-raw"></a>

## STR_VALUE_RAW

🟠 **MACRO** · *protocol::http::v11*

```cpp
#define STR_VALUE_RAW(x) #x
```

Provides common-usage macros.

**Value:** `#x`

**Source:** [include/protocol/http/v11/status_lines.h:41](../../../../../include/protocol/http/v11/status_lines.h#L41)

<a id="macro-str-value"></a>

## STR_VALUE

🟠 **MACRO** · *protocol::http::v11*

```cpp
#define STR_VALUE(x) STR_VALUE_RAW(x)
```

Defines the str value preprocessor value.

**Value:** `STR_VALUE_RAW(x)`

**Source:** [include/protocol/http/v11/status_lines.h:42](../../../../../include/protocol/http/v11/status_lines.h#L42)

<a id="macro-eas"></a>

## EAS

🟠 **MACRO** · *protocol::http::v11*

```cpp
#define EAS(x) x
```

Defines the eas preprocessor value.

**Value:** `x`

**Source:** [include/protocol/http/v11/status_lines.h:43](../../../../../include/protocol/http/v11/status_lines.h#L43)

<a id="macro-sl"></a>

## SL

🟠 **MACRO** · *protocol::http::v11*

```cpp
#define SL(x) "HTTP/1.1 " STR_VALUE(SC_##x) " " STR_VALUE(RP_##x) "\r\n"
```

Defines the sl preprocessor value.

**Value:** `"HTTP/1.1 " STR_VALUE(SC_##x) " " STR_VALUE(RP_##x) "\r\n"`

**Source:** [include/protocol/http/v11/status_lines.h:44](../../../../../include/protocol/http/v11/status_lines.h#L44)

<a id="type-status-lines"></a>

## status_lines

🟠 **TYPE** · *protocol::http::v11*

**Declaration synopsis:**

```cpp
struct status_lines { ... };
```

Provides the http 1.1 status lines.

**Source:** [include/protocol/http/v11/status_lines.h:53](../../../../../include/protocol/http/v11/status_lines.h#L53)

<a id="constant-k100"></a>

## k100

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k100[] = EAS(SL(100_CONTINUE));
```

Defines the 100 constant.

**Value:** `EAS(SL(100_CONTINUE))`

**Source:** [include/protocol/http/v11/status_lines.h:57](../../../../../include/protocol/http/v11/status_lines.h#L57)

<a id="constant-k101"></a>

## k101

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k101[] = EAS(SL(101_SWITCHING_PROTOCOLS));
```

Defines the 101 constant.

**Value:** `EAS(SL(101_SWITCHING_PROTOCOLS))`

**Source:** [include/protocol/http/v11/status_lines.h:58](../../../../../include/protocol/http/v11/status_lines.h#L58)

<a id="constant-k200"></a>

## k200

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k200[] = EAS(SL(200_OK));
```

Defines the 200 constant.

**Value:** `EAS(SL(200_OK))`

**Source:** [include/protocol/http/v11/status_lines.h:59](../../../../../include/protocol/http/v11/status_lines.h#L59)

<a id="constant-k201"></a>

## k201

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k201[] = EAS(SL(201_CREATED));
```

Defines the 201 constant.

**Value:** `EAS(SL(201_CREATED))`

**Source:** [include/protocol/http/v11/status_lines.h:60](../../../../../include/protocol/http/v11/status_lines.h#L60)

<a id="constant-k202"></a>

## k202

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k202[] = EAS(SL(202_ACCEPTED));
```

Defines the 202 constant.

**Value:** `EAS(SL(202_ACCEPTED))`

**Source:** [include/protocol/http/v11/status_lines.h:61](../../../../../include/protocol/http/v11/status_lines.h#L61)

<a id="constant-k203"></a>

## k203

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k203[] = EAS(SL(203_NON_AUTHORITATIVE_INFORMATION));
```

Defines the 203 constant.

**Value:** `EAS(SL(203_NON_AUTHORITATIVE_INFORMATION))`

**Source:** [include/protocol/http/v11/status_lines.h:62](../../../../../include/protocol/http/v11/status_lines.h#L62)

<a id="constant-k204"></a>

## k204

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k204[] = EAS(SL(204_NO_CONTENT));
```

Defines the 204 constant.

**Value:** `EAS(SL(204_NO_CONTENT))`

**Source:** [include/protocol/http/v11/status_lines.h:63](../../../../../include/protocol/http/v11/status_lines.h#L63)

<a id="constant-k205"></a>

## k205

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k205[] = EAS(SL(205_RESET_CONTENT));
```

Defines the 205 constant.

**Value:** `EAS(SL(205_RESET_CONTENT))`

**Source:** [include/protocol/http/v11/status_lines.h:64](../../../../../include/protocol/http/v11/status_lines.h#L64)

<a id="constant-k206"></a>

## k206

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k206[] = EAS(SL(206_PARTIAL_CONTENT));
```

Defines the 206 constant.

**Value:** `EAS(SL(206_PARTIAL_CONTENT))`

**Source:** [include/protocol/http/v11/status_lines.h:65](../../../../../include/protocol/http/v11/status_lines.h#L65)

<a id="constant-k300"></a>

## k300

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k300[] = EAS(SL(300_MULTIPLE_CHOICES));
```

Defines the 300 constant.

**Value:** `EAS(SL(300_MULTIPLE_CHOICES))`

**Source:** [include/protocol/http/v11/status_lines.h:66](../../../../../include/protocol/http/v11/status_lines.h#L66)

<a id="constant-k301"></a>

## k301

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k301[] = EAS(SL(301_MOVED_PERMANENTLY));
```

Defines the 301 constant.

**Value:** `EAS(SL(301_MOVED_PERMANENTLY))`

**Source:** [include/protocol/http/v11/status_lines.h:67](../../../../../include/protocol/http/v11/status_lines.h#L67)

<a id="constant-k302"></a>

## k302

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k302[] = EAS(SL(302_FOUND));
```

Defines the 302 constant.

**Value:** `EAS(SL(302_FOUND))`

**Source:** [include/protocol/http/v11/status_lines.h:68](../../../../../include/protocol/http/v11/status_lines.h#L68)

<a id="constant-k303"></a>

## k303

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k303[] = EAS(SL(303_SEE_OTHER));
```

Defines the 303 constant.

**Value:** `EAS(SL(303_SEE_OTHER))`

**Source:** [include/protocol/http/v11/status_lines.h:69](../../../../../include/protocol/http/v11/status_lines.h#L69)

<a id="constant-k304"></a>

## k304

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k304[] = EAS(SL(304_NOT_MODIFIED));
```

Defines the 304 constant.

**Value:** `EAS(SL(304_NOT_MODIFIED))`

**Source:** [include/protocol/http/v11/status_lines.h:70](../../../../../include/protocol/http/v11/status_lines.h#L70)

<a id="constant-k305"></a>

## k305

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k305[] = EAS(SL(305_USE_PROXY));
```

Defines the 305 constant.

**Value:** `EAS(SL(305_USE_PROXY))`

**Source:** [include/protocol/http/v11/status_lines.h:71](../../../../../include/protocol/http/v11/status_lines.h#L71)

<a id="constant-k306"></a>

## k306

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k306[] = EAS(SL(306_UNUSED));
```

Defines the 306 constant.

**Value:** `EAS(SL(306_UNUSED))`

**Source:** [include/protocol/http/v11/status_lines.h:72](../../../../../include/protocol/http/v11/status_lines.h#L72)

<a id="constant-k307"></a>

## k307

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k307[] = EAS(SL(307_TEMPORARY_REDIRECT));
```

Defines the 307 constant.

**Value:** `EAS(SL(307_TEMPORARY_REDIRECT))`

**Source:** [include/protocol/http/v11/status_lines.h:73](../../../../../include/protocol/http/v11/status_lines.h#L73)

<a id="constant-k308"></a>

## k308

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k308[] = EAS(SL(308_PERMANENT_REDIRECT));
```

Defines the 308 constant.

**Value:** `EAS(SL(308_PERMANENT_REDIRECT))`

**Source:** [include/protocol/http/v11/status_lines.h:74](../../../../../include/protocol/http/v11/status_lines.h#L74)

<a id="constant-k400"></a>

## k400

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k400[] = EAS(SL(400_BAD_REQUEST));
```

Defines the 400 constant.

**Value:** `EAS(SL(400_BAD_REQUEST))`

**Source:** [include/protocol/http/v11/status_lines.h:75](../../../../../include/protocol/http/v11/status_lines.h#L75)

<a id="constant-k401"></a>

## k401

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k401[] = EAS(SL(401_UNAUTHORIZED));
```

Defines the 401 constant.

**Value:** `EAS(SL(401_UNAUTHORIZED))`

**Source:** [include/protocol/http/v11/status_lines.h:76](../../../../../include/protocol/http/v11/status_lines.h#L76)

<a id="constant-k402"></a>

## k402

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k402[] = EAS(SL(402_PAYMENT_REQUIRED));
```

Defines the 402 constant.

**Value:** `EAS(SL(402_PAYMENT_REQUIRED))`

**Source:** [include/protocol/http/v11/status_lines.h:77](../../../../../include/protocol/http/v11/status_lines.h#L77)

<a id="constant-k403"></a>

## k403

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k403[] = EAS(SL(403_FORBIDDEN));
```

Defines the 403 constant.

**Value:** `EAS(SL(403_FORBIDDEN))`

**Source:** [include/protocol/http/v11/status_lines.h:78](../../../../../include/protocol/http/v11/status_lines.h#L78)

<a id="constant-k404"></a>

## k404

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k404[] = EAS(SL(404_NOT_FOUND));
```

Defines the 404 constant.

**Value:** `EAS(SL(404_NOT_FOUND))`

**Source:** [include/protocol/http/v11/status_lines.h:79](../../../../../include/protocol/http/v11/status_lines.h#L79)

<a id="constant-k405"></a>

## k405

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k405[] = EAS(SL(405_METHOD_NOT_ALLOWED));
```

Defines the 405 constant.

**Value:** `EAS(SL(405_METHOD_NOT_ALLOWED))`

**Source:** [include/protocol/http/v11/status_lines.h:80](../../../../../include/protocol/http/v11/status_lines.h#L80)

<a id="constant-k406"></a>

## k406

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k406[] = EAS(SL(406_NOT_ACCEPTABLE));
```

Defines the 406 constant.

**Value:** `EAS(SL(406_NOT_ACCEPTABLE))`

**Source:** [include/protocol/http/v11/status_lines.h:81](../../../../../include/protocol/http/v11/status_lines.h#L81)

<a id="constant-k407"></a>

## k407

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k407[] = EAS(SL(407_PROXY_AUTHENTICATION_REQUIRED));
```

Defines the 407 constant.

**Value:** `EAS(SL(407_PROXY_AUTHENTICATION_REQUIRED))`

**Source:** [include/protocol/http/v11/status_lines.h:82](../../../../../include/protocol/http/v11/status_lines.h#L82)

<a id="constant-k408"></a>

## k408

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k408[] = EAS(SL(408_REQUEST_TIMEOUT));
```

Defines the 408 constant.

**Value:** `EAS(SL(408_REQUEST_TIMEOUT))`

**Source:** [include/protocol/http/v11/status_lines.h:83](../../../../../include/protocol/http/v11/status_lines.h#L83)

<a id="constant-k409"></a>

## k409

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k409[] = EAS(SL(409_CONFLICT));
```

Defines the 409 constant.

**Value:** `EAS(SL(409_CONFLICT))`

**Source:** [include/protocol/http/v11/status_lines.h:84](../../../../../include/protocol/http/v11/status_lines.h#L84)

<a id="constant-k410"></a>

## k410

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k410[] = EAS(SL(410_GONE));
```

Defines the 410 constant.

**Value:** `EAS(SL(410_GONE))`

**Source:** [include/protocol/http/v11/status_lines.h:85](../../../../../include/protocol/http/v11/status_lines.h#L85)

<a id="constant-k411"></a>

## k411

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k411[] = EAS(SL(411_LENGTH_REQUIRED));
```

Defines the 411 constant.

**Value:** `EAS(SL(411_LENGTH_REQUIRED))`

**Source:** [include/protocol/http/v11/status_lines.h:86](../../../../../include/protocol/http/v11/status_lines.h#L86)

<a id="constant-k412"></a>

## k412

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k412[] = EAS(SL(412_PRECONDITION_FAILED));
```

Defines the 412 constant.

**Value:** `EAS(SL(412_PRECONDITION_FAILED))`

**Source:** [include/protocol/http/v11/status_lines.h:87](../../../../../include/protocol/http/v11/status_lines.h#L87)

<a id="constant-k413"></a>

## k413

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k413[] = EAS(SL(413_CONTENT_TOO_LARGE));
```

Defines the 413 constant.

**Value:** `EAS(SL(413_CONTENT_TOO_LARGE))`

**Source:** [include/protocol/http/v11/status_lines.h:88](../../../../../include/protocol/http/v11/status_lines.h#L88)

<a id="constant-k414"></a>

## k414

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k414[] = EAS(SL(414_URI_TOO_LONG));
```

Defines the 414 constant.

**Value:** `EAS(SL(414_URI_TOO_LONG))`

**Source:** [include/protocol/http/v11/status_lines.h:89](../../../../../include/protocol/http/v11/status_lines.h#L89)

<a id="constant-k415"></a>

## k415

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k415[] = EAS(SL(415_UNSUPPORTED_MEDIA_TYPE));
```

Defines the 415 constant.

**Value:** `EAS(SL(415_UNSUPPORTED_MEDIA_TYPE))`

**Source:** [include/protocol/http/v11/status_lines.h:90](../../../../../include/protocol/http/v11/status_lines.h#L90)

<a id="constant-k416"></a>

## k416

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k416[] = EAS(SL(416_RANGE_NOT_SATISFIABLE));
```

Defines the 416 constant.

**Value:** `EAS(SL(416_RANGE_NOT_SATISFIABLE))`

**Source:** [include/protocol/http/v11/status_lines.h:91](../../../../../include/protocol/http/v11/status_lines.h#L91)

<a id="constant-k417"></a>

## k417

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k417[] = EAS(SL(417_EXPECTATION_FAILED));
```

Defines the 417 constant.

**Value:** `EAS(SL(417_EXPECTATION_FAILED))`

**Source:** [include/protocol/http/v11/status_lines.h:92](../../../../../include/protocol/http/v11/status_lines.h#L92)

<a id="constant-k418"></a>

## k418

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k418[] = EAS(SL(418_IM_A_TEAPOT));
```

Defines the 418 constant.

**Value:** `EAS(SL(418_IM_A_TEAPOT))`

**Source:** [include/protocol/http/v11/status_lines.h:93](../../../../../include/protocol/http/v11/status_lines.h#L93)

<a id="constant-k421"></a>

## k421

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k421[] = EAS(SL(421_MISDIRECTED_REQUEST));
```

Defines the 421 constant.

**Value:** `EAS(SL(421_MISDIRECTED_REQUEST))`

**Source:** [include/protocol/http/v11/status_lines.h:94](../../../../../include/protocol/http/v11/status_lines.h#L94)

<a id="constant-k422"></a>

## k422

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k422[] = EAS(SL(422_UNPROCESSABLE_CONTENT));
```

Defines the 422 constant.

**Value:** `EAS(SL(422_UNPROCESSABLE_CONTENT))`

**Source:** [include/protocol/http/v11/status_lines.h:95](../../../../../include/protocol/http/v11/status_lines.h#L95)

<a id="constant-k426"></a>

## k426

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k426[] = EAS(SL(426_UPGRADE_REQUIRED));
```

Defines the 426 constant.

**Value:** `EAS(SL(426_UPGRADE_REQUIRED))`

**Source:** [include/protocol/http/v11/status_lines.h:96](../../../../../include/protocol/http/v11/status_lines.h#L96)

<a id="constant-k431"></a>

## k431

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k431[] = EAS(SL(431_REQUEST_HEADER_FIELDS_TOO_LARGE));
```

Defines the 431 constant.

**Value:** `EAS(SL(431_REQUEST_HEADER_FIELDS_TOO_LARGE))`

**Source:** [include/protocol/http/v11/status_lines.h:97](../../../../../include/protocol/http/v11/status_lines.h#L97)

<a id="constant-k500"></a>

## k500

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k500[] = EAS(SL(500_INTERNAL_SERVER_ERROR));
```

Defines the 500 constant.

**Value:** `EAS(SL(500_INTERNAL_SERVER_ERROR))`

**Source:** [include/protocol/http/v11/status_lines.h:98](../../../../../include/protocol/http/v11/status_lines.h#L98)

<a id="constant-k501"></a>

## k501

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k501[] = EAS(SL(501_NOT_IMPLEMENTED));
```

Defines the 501 constant.

**Value:** `EAS(SL(501_NOT_IMPLEMENTED))`

**Source:** [include/protocol/http/v11/status_lines.h:99](../../../../../include/protocol/http/v11/status_lines.h#L99)

<a id="constant-k502"></a>

## k502

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k502[] = EAS(SL(502_BAD_GATEWAY));
```

Defines the 502 constant.

**Value:** `EAS(SL(502_BAD_GATEWAY))`

**Source:** [include/protocol/http/v11/status_lines.h:100](../../../../../include/protocol/http/v11/status_lines.h#L100)

<a id="constant-k503"></a>

## k503

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k503[] = EAS(SL(503_SERVICE_UNAVAILABLE));
```

Defines the 503 constant.

**Value:** `EAS(SL(503_SERVICE_UNAVAILABLE))`

**Source:** [include/protocol/http/v11/status_lines.h:101](../../../../../include/protocol/http/v11/status_lines.h#L101)

<a id="constant-k504"></a>

## k504

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k504[] = EAS(SL(504_GATEWAY_TIMEOUT));
```

Defines the 504 constant.

**Value:** `EAS(SL(504_GATEWAY_TIMEOUT))`

**Source:** [include/protocol/http/v11/status_lines.h:102](../../../../../include/protocol/http/v11/status_lines.h#L102)

<a id="constant-k505"></a>

## k505

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr char k505[] = EAS(SL(505_HTTP_VERSION_NOT_SUPPORTED));
```

Defines the 505 constant.

**Value:** `EAS(SL(505_HTTP_VERSION_NOT_SUPPORTED))`

**Source:** [include/protocol/http/v11/status_lines.h:103](../../../../../include/protocol/http/v11/status_lines.h#L103)

<a id="constant-k100sz"></a>

## k100Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k100Sz = sizeof(k100) - 1;
```

Defines the 100 sz constant.

**Value:** `sizeof(k100) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:107](../../../../../include/protocol/http/v11/status_lines.h#L107)

<a id="constant-k101sz"></a>

## k101Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k101Sz = sizeof(k101) - 1;
```

Defines the 101 sz constant.

**Value:** `sizeof(k101) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:108](../../../../../include/protocol/http/v11/status_lines.h#L108)

<a id="constant-k200sz"></a>

## k200Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k200Sz = sizeof(k200) - 1;
```

Defines the 200 sz constant.

**Value:** `sizeof(k200) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:109](../../../../../include/protocol/http/v11/status_lines.h#L109)

<a id="constant-k201sz"></a>

## k201Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k201Sz = sizeof(k201) - 1;
```

Defines the 201 sz constant.

**Value:** `sizeof(k201) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:110](../../../../../include/protocol/http/v11/status_lines.h#L110)

<a id="constant-k202sz"></a>

## k202Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k202Sz = sizeof(k202) - 1;
```

Defines the 202 sz constant.

**Value:** `sizeof(k202) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:111](../../../../../include/protocol/http/v11/status_lines.h#L111)

<a id="constant-k203sz"></a>

## k203Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k203Sz = sizeof(k203) - 1;
```

Defines the 203 sz constant.

**Value:** `sizeof(k203) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:112](../../../../../include/protocol/http/v11/status_lines.h#L112)

<a id="constant-k204sz"></a>

## k204Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k204Sz = sizeof(k204) - 1;
```

Defines the 204 sz constant.

**Value:** `sizeof(k204) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:113](../../../../../include/protocol/http/v11/status_lines.h#L113)

<a id="constant-k205sz"></a>

## k205Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k205Sz = sizeof(k205) - 1;
```

Defines the 205 sz constant.

**Value:** `sizeof(k205) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:114](../../../../../include/protocol/http/v11/status_lines.h#L114)

<a id="constant-k206sz"></a>

## k206Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k206Sz = sizeof(k206) - 1;
```

Defines the 206 sz constant.

**Value:** `sizeof(k206) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:115](../../../../../include/protocol/http/v11/status_lines.h#L115)

<a id="constant-k300sz"></a>

## k300Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k300Sz = sizeof(k300) - 1;
```

Defines the 300 sz constant.

**Value:** `sizeof(k300) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:116](../../../../../include/protocol/http/v11/status_lines.h#L116)

<a id="constant-k301sz"></a>

## k301Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k301Sz = sizeof(k301) - 1;
```

Defines the 301 sz constant.

**Value:** `sizeof(k301) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:117](../../../../../include/protocol/http/v11/status_lines.h#L117)

<a id="constant-k302sz"></a>

## k302Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k302Sz = sizeof(k302) - 1;
```

Defines the 302 sz constant.

**Value:** `sizeof(k302) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:118](../../../../../include/protocol/http/v11/status_lines.h#L118)

<a id="constant-k303sz"></a>

## k303Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k303Sz = sizeof(k303) - 1;
```

Defines the 303 sz constant.

**Value:** `sizeof(k303) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:119](../../../../../include/protocol/http/v11/status_lines.h#L119)

<a id="constant-k304sz"></a>

## k304Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k304Sz = sizeof(k304) - 1;
```

Defines the 304 sz constant.

**Value:** `sizeof(k304) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:120](../../../../../include/protocol/http/v11/status_lines.h#L120)

<a id="constant-k305sz"></a>

## k305Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k305Sz = sizeof(k305) - 1;
```

Defines the 305 sz constant.

**Value:** `sizeof(k305) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:121](../../../../../include/protocol/http/v11/status_lines.h#L121)

<a id="constant-k306sz"></a>

## k306Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k306Sz = sizeof(k306) - 1;
```

Defines the 306 sz constant.

**Value:** `sizeof(k306) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:122](../../../../../include/protocol/http/v11/status_lines.h#L122)

<a id="constant-k307sz"></a>

## k307Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k307Sz = sizeof(k307) - 1;
```

Defines the 307 sz constant.

**Value:** `sizeof(k307) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:123](../../../../../include/protocol/http/v11/status_lines.h#L123)

<a id="constant-k308sz"></a>

## k308Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k308Sz = sizeof(k308) - 1;
```

Defines the 308 sz constant.

**Value:** `sizeof(k308) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:124](../../../../../include/protocol/http/v11/status_lines.h#L124)

<a id="constant-k400sz"></a>

## k400Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k400Sz = sizeof(k400) - 1;
```

Defines the 400 sz constant.

**Value:** `sizeof(k400) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:125](../../../../../include/protocol/http/v11/status_lines.h#L125)

<a id="constant-k401sz"></a>

## k401Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k401Sz = sizeof(k401) - 1;
```

Defines the 401 sz constant.

**Value:** `sizeof(k401) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:126](../../../../../include/protocol/http/v11/status_lines.h#L126)

<a id="constant-k402sz"></a>

## k402Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k402Sz = sizeof(k402) - 1;
```

Defines the 402 sz constant.

**Value:** `sizeof(k402) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:127](../../../../../include/protocol/http/v11/status_lines.h#L127)

<a id="constant-k403sz"></a>

## k403Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k403Sz = sizeof(k403) - 1;
```

Defines the 403 sz constant.

**Value:** `sizeof(k403) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:128](../../../../../include/protocol/http/v11/status_lines.h#L128)

<a id="constant-k404sz"></a>

## k404Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k404Sz = sizeof(k404) - 1;
```

Defines the 404 sz constant.

**Value:** `sizeof(k404) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:129](../../../../../include/protocol/http/v11/status_lines.h#L129)

<a id="constant-k405sz"></a>

## k405Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k405Sz = sizeof(k405) - 1;
```

Defines the 405 sz constant.

**Value:** `sizeof(k405) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:130](../../../../../include/protocol/http/v11/status_lines.h#L130)

<a id="constant-k406sz"></a>

## k406Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k406Sz = sizeof(k406) - 1;
```

Defines the 406 sz constant.

**Value:** `sizeof(k406) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:131](../../../../../include/protocol/http/v11/status_lines.h#L131)

<a id="constant-k407sz"></a>

## k407Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k407Sz = sizeof(k407) - 1;
```

Defines the 407 sz constant.

**Value:** `sizeof(k407) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:132](../../../../../include/protocol/http/v11/status_lines.h#L132)

<a id="constant-k408sz"></a>

## k408Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k408Sz = sizeof(k408) - 1;
```

Defines the 408 sz constant.

**Value:** `sizeof(k408) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:133](../../../../../include/protocol/http/v11/status_lines.h#L133)

<a id="constant-k409sz"></a>

## k409Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k409Sz = sizeof(k409) - 1;
```

Defines the 409 sz constant.

**Value:** `sizeof(k409) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:134](../../../../../include/protocol/http/v11/status_lines.h#L134)

<a id="constant-k410sz"></a>

## k410Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k410Sz = sizeof(k410) - 1;
```

Defines the 410 sz constant.

**Value:** `sizeof(k410) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:135](../../../../../include/protocol/http/v11/status_lines.h#L135)

<a id="constant-k411sz"></a>

## k411Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k411Sz = sizeof(k411) - 1;
```

Defines the 411 sz constant.

**Value:** `sizeof(k411) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:136](../../../../../include/protocol/http/v11/status_lines.h#L136)

<a id="constant-k412sz"></a>

## k412Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k412Sz = sizeof(k412) - 1;
```

Defines the 412 sz constant.

**Value:** `sizeof(k412) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:137](../../../../../include/protocol/http/v11/status_lines.h#L137)

<a id="constant-k413sz"></a>

## k413Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k413Sz = sizeof(k413) - 1;
```

Defines the 413 sz constant.

**Value:** `sizeof(k413) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:138](../../../../../include/protocol/http/v11/status_lines.h#L138)

<a id="constant-k414sz"></a>

## k414Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k414Sz = sizeof(k414) - 1;
```

Defines the 414 sz constant.

**Value:** `sizeof(k414) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:139](../../../../../include/protocol/http/v11/status_lines.h#L139)

<a id="constant-k415sz"></a>

## k415Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k415Sz = sizeof(k415) - 1;
```

Defines the 415 sz constant.

**Value:** `sizeof(k415) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:140](../../../../../include/protocol/http/v11/status_lines.h#L140)

<a id="constant-k416sz"></a>

## k416Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k416Sz = sizeof(k416) - 1;
```

Defines the 416 sz constant.

**Value:** `sizeof(k416) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:141](../../../../../include/protocol/http/v11/status_lines.h#L141)

<a id="constant-k417sz"></a>

## k417Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k417Sz = sizeof(k417) - 1;
```

Defines the 417 sz constant.

**Value:** `sizeof(k417) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:142](../../../../../include/protocol/http/v11/status_lines.h#L142)

<a id="constant-k418sz"></a>

## k418Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k418Sz = sizeof(k418) - 1;
```

Defines the 418 sz constant.

**Value:** `sizeof(k418) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:143](../../../../../include/protocol/http/v11/status_lines.h#L143)

<a id="constant-k421sz"></a>

## k421Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k421Sz = sizeof(k421) - 1;
```

Defines the 421 sz constant.

**Value:** `sizeof(k421) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:144](../../../../../include/protocol/http/v11/status_lines.h#L144)

<a id="constant-k422sz"></a>

## k422Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k422Sz = sizeof(k422) - 1;
```

Defines the 422 sz constant.

**Value:** `sizeof(k422) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:145](../../../../../include/protocol/http/v11/status_lines.h#L145)

<a id="constant-k426sz"></a>

## k426Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k426Sz = sizeof(k426) - 1;
```

Defines the 426 sz constant.

**Value:** `sizeof(k426) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:146](../../../../../include/protocol/http/v11/status_lines.h#L146)

<a id="constant-k431sz"></a>

## k431Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k431Sz = sizeof(k431) - 1;
```

Defines the 431 sz constant.

**Value:** `sizeof(k431) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:147](../../../../../include/protocol/http/v11/status_lines.h#L147)

<a id="constant-k500sz"></a>

## k500Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k500Sz = sizeof(k500) - 1;
```

Defines the 500 sz constant.

**Value:** `sizeof(k500) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:148](../../../../../include/protocol/http/v11/status_lines.h#L148)

<a id="constant-k501sz"></a>

## k501Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k501Sz = sizeof(k501) - 1;
```

Defines the 501 sz constant.

**Value:** `sizeof(k501) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:149](../../../../../include/protocol/http/v11/status_lines.h#L149)

<a id="constant-k502sz"></a>

## k502Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k502Sz = sizeof(k502) - 1;
```

Defines the 502 sz constant.

**Value:** `sizeof(k502) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:150](../../../../../include/protocol/http/v11/status_lines.h#L150)

<a id="constant-k503sz"></a>

## k503Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k503Sz = sizeof(k503) - 1;
```

Defines the 503 sz constant.

**Value:** `sizeof(k503) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:151](../../../../../include/protocol/http/v11/status_lines.h#L151)

<a id="constant-k504sz"></a>

## k504Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k504Sz = sizeof(k504) - 1;
```

Defines the 504 sz constant.

**Value:** `sizeof(k504) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:152](../../../../../include/protocol/http/v11/status_lines.h#L152)

<a id="constant-k505sz"></a>

## k505Sz

🟠 **CONSTANT** · *protocol::http::v11::status_lines*

```cpp
static constexpr std::size_t k505Sz = sizeof(k505) - 1;
```

Defines the 505 sz constant.

**Value:** `sizeof(k505) - 1`

**Source:** [include/protocol/http/v11/status_lines.h:153](../../../../../include/protocol/http/v11/status_lines.h#L153)
