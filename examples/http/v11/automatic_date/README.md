# Automatic date

This example does not set a `Date` field. Response serialization adds the
current HTTP date automatically.

## Try it

```text
curl -i http://localhost:8080/date
```

The response includes `Date` in IMF-fixdate format.

