#!/usr/bin/env python3
import urllib.request
import urllib.parse
import json
import sys

def fetch_wiki(topic):
    url = f"https://en.wikipedia.org/api/rest_v1/page/summary/{urllib.parse.quote(topic)}"
    try:
        req = urllib.request.Request(url, headers={'User-Agent': 'AntigravitySkill'})
        with urllib.request.urlopen(req) as response:
            data = json.loads(response.read().decode())
            if 'extract' in data:
                print(f"--- Wikipedia Summary for {topic} ---")
                print(data['extract'])
            else:
                print(f"No Wikipedia summary found for {topic}.")
    except urllib.error.HTTPError as e:
        if e.code == 404:
             print(f"Topic '{topic}' not found on Wikipedia.")
        else:
             print(f"HTTP Error: {e.code}")
    except Exception as e:
        print(f"Could not fetch Wikipedia: {e}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: fetch_wiki.py <topic>")
        sys.exit(1)
    topic = " ".join(sys.argv[1:])
    fetch_wiki(topic)
