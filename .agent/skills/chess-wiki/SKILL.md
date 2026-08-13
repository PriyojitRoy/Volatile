---
name: chess-wiki
description: Fetch chess concepts from Wikipedia and present a technical breakdown.
---

# Chess Wiki Skill

This skill allows you to quickly fetch summaries of chess and chess programming concepts.

## Usage

When the user asks for `/chess-wiki <topic>`, you should execute the `fetch_wiki.py` script located in this same directory, passing the topic as an argument.

```bash
python3 /home/priyojitroy2005/Desktop/Volatile/.agent/skills/chess-wiki/fetch_wiki.py "<topic>"
```

After fetching the data, present the clean summary and any technical breakdown directly to the user in the terminal response.
