from urllib.parse import urlparse

url = "https://123.123.12.23/something/else"
parsed = urlparse(url)

base_url = f"{parsed.scheme}://{parsed.hostname}"
print(base_url)  # Output: http://123.123.12.23