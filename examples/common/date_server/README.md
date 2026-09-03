# Date server

This example starts the process-wide HTTP-date cache, reads its current
29-character IMF-fixdate value, then stops it. Both lifecycle calls are
idempotent.

## Run

Run `common_date_server`. It prints a current HTTP date such as
`Sun, 06 Nov 1994 08:49:37 GMT`.

