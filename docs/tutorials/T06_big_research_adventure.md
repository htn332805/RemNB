# T06 — The Big Research Adventure 🏆🌍

**Time:** 45–60 minutes  
**Difficulty:** ⭐⭐⭐⭐ (you are using everything you learned!)  
**Builds on:** ALL previous tutorials T00–T05  
**Goal:** Take a completely new topic and build a full, linked, AI-assisted knowledge vault section from scratch — end to end.

---

## What Will I Learn?

By the end of this tutorial you will know:
- ✅ How to run the **complete workflow** from raw document → vault → AI context
- ✅ How to organise a multi-note topic cluster in your vault
- ✅ How Qdrant grows alongside your vault
- ✅ How to write a "Research Summary" note that caps off a topic

---

## Your Mission (Choose ONE)

Pick the topic you find most interesting. You will research it using only the document you create:

| Option | Topic |
|---|---|
| 🚀 A | **Space Exploration** — rockets, planets, astronauts |
| 🦕 B | **Dinosaurs** — types, extinction, fossils |
| 🌿 C | **Plants** — photosynthesis, rainforests, strange plants |
| 🤖 D | **Robots and AI** — history, famous robots, how AI works |
| ⚡ E | **Electricity** — how it works, generators, lightning |
| 🐝 F | **Insects** — bees, ants, butterflies, ecosystems |

For this tutorial, the steps use **Space Exploration** as the example. If you chose a different topic, just replace "space" with your topic!

---

## Phase 1 — Create Your Research Document (5 min)

### Step 1 — Create the document

```bash
cd /home/tester/aios
mkdir -p workspace/my_documents

cat > workspace/my_documents/space_exploration.txt << 'ENDOFFILE'
# Space Exploration — Key Facts

Space exploration is the investigation of outer space using spacecraft, telescopes, and satellites.
Humans first reached space in 1961 when Soviet cosmonaut Yuri Gagarin orbited Earth.

## Rockets
Rockets work using Newton's Third Law: every action has an equal and opposite reaction.
Burning fuel shoots downward, pushing the rocket upward.
Modern rockets like SpaceX's Falcon 9 can land back on Earth and be reused.
The Saturn V rocket that took humans to the Moon remains the most powerful rocket ever built.

## The Moon Landing
On July 20, 1969, Apollo 11 landed on the Moon.
Neil Armstrong and Buzz Aldrin walked on the surface while Michael Collins orbited above.
The Moon is 384,400 kilometres from Earth.
Astronauts left laser reflectors on the Moon that scientists still use today.

## Mars
Mars is the most likely planet for future human colonisation.
It is about half the size of Earth and takes 687 days to orbit the Sun.
NASA's Perseverance rover landed on Mars in February 2021.
Scientists are looking for signs of ancient microbial life in Martian rock samples.

## Satellites and ISS
The International Space Station (ISS) has been continuously inhabited since the year 2000.
It orbits Earth at about 400 kilometres altitude at 28,000 km/h.
Over 250 astronauts from 20 different countries have visited the ISS.
Thousands of satellites orbit Earth providing GPS, weather data, and internet.

## Telescopes
The Hubble Space Telescope has been operating since 1990.
The James Webb Space Telescope (JWST), launched in 2021, can see galaxies that formed
just 300 million years after the Big Bang — nearly 13.5 billion years ago.

ENDOFFILE
echo "Document created!"
```

---

## Phase 2 — AI Analysis (5 min)

### Step 2 — Run the analysis

```bash
aios-vault analyze workspace/my_documents/space_exploration.txt
```

### Step 3 — Save the analysis

```bash
aios-vault analyze workspace/my_documents/space_exploration.txt \
  > workspace/my_documents/space_exploration_analysis.txt

echo "=== Analysis saved! Key topics the AI found: ==="
grep -A 10 "Key Topics" workspace/my_documents/space_exploration_analysis.txt
```

Note down the **Key Topics** the AI listed. You will create one vault note for each.

---

## Phase 3 — Build Your Vault Topic Cluster (20–25 min)

You are going to create **5 notes** + 1 overview note. That is a full topic cluster!

### Step 4 — Create the first note: Rockets

```bash
cat > /home/tester/aios/obsidian/vault/Rockets.md << 'ENDOFNOTE'
---
title: Rockets
tags: [space, technology, physics, propulsion]
topic: space exploration
created: 2026-03-12
source: space_exploration.txt
---

# Rockets

Rockets make space travel possible by producing enormous thrust.

## How Rockets Work
Rockets use **Newton's Third Law**: push gas down → rocket goes up.
Burning fuel shoots out the bottom at high speed.
The bigger the rocket, the more fuel it needs.

## Famous Rockets
- **Saturn V** — most powerful rocket ever; used for Moon landings
- **Falcon 9** (SpaceX) — first reusable orbital rocket
- **SLS** — NASA's current mega-rocket for deep space missions

## Reusability
Traditional rockets were thrown away after each launch.
SpaceX's Falcon 9 lands back on Earth (or on a ship at sea!) and flies again.
This makes space exploration much cheaper.

## Related Notes
- [[Space Exploration Overview]]
- [[Moon Landing 1969]]
- [[Physics of Motion]]

ENDOFNOTE
```

### Step 5 — Create the Moon Landing note

```bash
cat > /home/tester/aios/obsidian/vault/Moon_Landing_1969.md << 'ENDOFNOTE'
---
title: Moon Landing 1969
tags: [space, history, NASA, Apollo, Moon]
topic: space exploration
created: 2026-03-12
source: space_exploration.txt
---

# Moon Landing 1969

The Apollo 11 mission was humanity's first landing on another world.

## Key Facts
- **Date:** 20 July 1969
- **First human on Moon:** Neil Armstrong
- **Also walked on Moon:** Buzz Aldrin
- **Stayed in orbit:** Michael Collins
- **Distance to Moon:** 384,400 km
- **Travel time:** about 3 days each way

## What They Left Behind
Astronauts placed **laser reflectors** on the surface.
Scientists today still bounce lasers off them to precisely measure the Moon's distance.

## Why It Mattered
It proved humans could survive in space long enough to reach another world.
It started our understanding of the Moon's geology.

## Related Notes
- [[Space Exploration Overview]]
- [[Rockets]]
- [[International Space Station]]

ENDOFNOTE
```

### Step 6 — Create the Mars note

```bash
cat > /home/tester/aios/obsidian/vault/Mars_Exploration.md << 'ENDOFNOTE'
---
title: Mars Exploration
tags: [space, Mars, planets, NASA, rovers]
topic: space exploration
created: 2026-03-12
source: space_exploration.txt
---

# Mars Exploration

Mars is the next great target for human space exploration.

## Mars Facts
- Size: about **half the size of Earth**
- Year length: **687 Earth days**
- Distance from Earth: between 56 and 401 million km (changes as both planets orbit)

## Current Missions
- **Perseverance rover** — landed February 2021
  - Collects rock samples to search for ancient microbial life
  - Generated oxygen from Martian atmosphere (MOXIE experiment)

## Challenges for Humans on Mars
- 7-month journey each way
- Very thin atmosphere (mostly CO2, very little oxygen)
- Extreme temperature swings (-80°C at night)
- High radiation due to no magnetic field

## Future Plans
SpaceX plans crewed missions to Mars in the late 2020s.

## Related Notes
- [[Space Exploration Overview]]
- [[Rockets]]
- [[Telescopes and Observation]]

ENDOFNOTE
```

### Step 7 — Create the ISS note

```bash
cat > /home/tester/aios/obsidian/vault/International_Space_Station.md << 'ENDOFNOTE'
---
title: International Space Station
tags: [space, ISS, history, astronauts, cooperation]
topic: space exploration
created: 2026-03-12
source: space_exploration.txt
---

# International Space Station (ISS)

The ISS is a permanent human outpost in low Earth orbit.

## Key Facts
- **Continuously inhabited since:** Year 2000 (25+ years!)
- **Orbit altitude:** ~400 km
- **Orbital speed:** 28,000 km/h (one lap of Earth every 90 minutes)
- **Total visitors:** over 250 astronauts from 20 countries

## Science on the ISS
Astronauts study how the human body changes in zero gravity.
Plants have been grown in space on the ISS.
Water and fire behave very differently in zero gravity — both are studied.

## International Cooperation
The ISS is jointly operated by NASA (USA), Roscosmos (Russia), ESA (Europe),
JAXA (Japan) and CSA (Canada). It is one of the greatest examples of
countries working together peacefully.

## Related Notes
- [[Space Exploration Overview]]
- [[Moon Landing 1969]]
- [[Mars Exploration]]

ENDOFNOTE
```

### Step 8 — Create the Telescopes note

```bash
cat > /home/tester/aios/obsidian/vault/Telescopes_and_Observation.md << 'ENDOFNOTE'
---
title: Telescopes and Observation
tags: [space, telescopes, astronomy, JWST, Hubble]
topic: space exploration
created: 2026-03-12
source: space_exploration.txt
---

# Telescopes and Observation

Telescopes let us see things in space billions of light-years away.

## Hubble Space Telescope
- Launched: **1990**
- Orbits Earth at 547 km altitude
- Has taken over 1 million observations
- Helped measure the **expansion rate of the universe**

## James Webb Space Telescope (JWST)
- Launched: December **2021**
- Orbits 1.5 million km from Earth (at L2 Lagrange point)
- Sees in **infrared** (heat light), not visible light
- Can see galaxies that formed **300 million years after the Big Bang**
  — looking back nearly **13.5 billion years**

## Why Space Telescopes Are Better
Earth's atmosphere blurs and blocks light. Space telescopes above the
atmosphere get a perfectly clear view.

## Related Notes
- [[Space Exploration Overview]]
- [[Mars Exploration]]
- [[Big Bang and Universe Origins]]

ENDOFNOTE
```

### Step 9 — Create the Overview Hub Note

```bash
cat > /home/tester/aios/obsidian/vault/Space_Exploration_Overview.md << 'ENDOFNOTE'
---
title: Space Exploration Overview
tags: [space, overview, history, technology]
topic: space exploration
created: 2026-03-12
source: space_exploration.txt
---

# Space Exploration Overview

This note is the **hub** for all space exploration topic notes.
Humans began exploring space in 1961 and have not stopped since.

## Timeline
- **1957** — Sputnik: first satellite in orbit (Soviet Union)
- **1961** — Yuri Gagarin: first human in space
- **1969** — [[Moon Landing 1969]]: first humans on the Moon (USA)
- **2000** — [[International Space Station]]: permanent human presence in orbit
- **2021** — [[Mars Exploration]]: Perseverance rover on Mars
- **2021** — [[Telescopes and Observation]]: James Webb Space Telescope launched

## Topic Notes
- [[Rockets]] — how we get to space
- [[Moon Landing 1969]] — humanity's greatest adventure so far
- [[Mars Exploration]] — the next frontier
- [[International Space Station]] — living in space right now
- [[Telescopes and Observation]] — looking deeper into the universe

## Cross-Topic Links
- [[Physics of Motion]] — the science behind rockets
- [[Big Bang and Universe Origins]] — what telescopes reveal about the universe

ENDOFNOTE
```

---

## Phase 4 — Index and Verify (5 min)

### Step 10 — Index your new notes into Qdrant

```bash
aios-vault review
```

### Step 11 — Check everything looks good

```bash
echo "=== VAULT CONTENTS ==="
ls /home/tester/aios/obsidian/vault/ | grep -i space

echo ""
echo "=== TOTAL LINKS IN SPACE NOTES ==="
grep -roh "\[\[[^\]]*\]\]" /home/tester/aios/obsidian/vault/ \
  | grep -i "space\|rocket\|moon\|mars\|ISS\|telescope" | wc -l

echo ""
echo "=== VAULT INSIGHTS (space section) ==="
aios-vault insights | head -40
```

---

## Phase 5 — AI + Context Query (10 min)

### Step 12 — Ask a Deep Question Using Your Vault as Context

Build a context package from your space notes:

```bash
cat /home/tester/aios/obsidian/vault/Space_Exploration_Overview.md \
    /home/tester/aios/obsidian/vault/Rockets.md \
    /home/tester/aios/obsidian/vault/Mars_Exploration.md \
    > /tmp/space_context.txt
```

Now ask a question that requires connecting multiple notes:

```bash
curl -s -X POST http://localhost:8080/agent/intent/classify \
  -H "Content-Type: application/json" \
  -d "{\"text\": \"Based on these notes about space, what makes Mars the most challenging destination compared to the Moon?\"}"
```

The intent should be `search_knowledge` with high confidence.

### Step 13 — Save a Research Summary Note

```bash
cat > /home/tester/aios/obsidian/vault/Space_Research_Summary.md << 'ENDOFNOTE'
---
title: Space Research Summary
tags: [space, summary, research-complete]
topic: space exploration
created: 2026-03-12
type: research-summary
---

# Space Research Summary

This note summarises my research into space exploration.

## What I Learned
1. Rockets work using physics (Newton's Third Law) and modern rockets can be reused
2. Humans landed on the Moon in 1969 — the biggest achievement in exploration history
3. Mars is the next goal but is far harder than the Moon due to distance, thin air, and radiation
4. The ISS has had humans living in space continuously since 2000
5. The JWST can see nearly 13.5 billion years into the past

## Most Surprising Fact
The octopus... wait, wrong vault topic! 😄
The most surprising space fact: the James Webb Telescope can see light that left
the universe when the universe was only 300 million years old. The universe is
now 13.8 billion years old. That is almost unimaginable!

## Questions I Still Have
- [[How Do Astronauts Exercise in Space?]]
- [[What Would a Mars Colony Look Like?]]
- [[How Does the JWST Stay So Cold?]]

## My Topic Notes
- [[Space Exploration Overview]]
- [[Rockets]]
- [[Moon Landing 1969]]
- [[Mars Exploration]]
- [[International Space Station]]
- [[Telescopes and Observation]]

ENDOFNOTE
```

---

## Phase 6 — Final Vault Check (5 min)

### Step 14 — The Grand Summary

```bash
echo "======================================"
echo "  YOUR VAULT AFTER THE ADVENTURE"
echo "======================================"
echo ""
echo "All notes:"
ls /home/tester/aios/obsidian/vault/ | nl

echo ""
echo "Total notes:"
ls /home/tester/aios/obsidian/vault/*.md | wc -l

echo ""
echo "Total links:"
grep -roh "\[\[[^\]]*\]\]" /home/tester/aios/obsidian/vault/ | wc -l

echo ""
echo "Topics covered:"
grep -rh "^topic:" /home/tester/aios/obsidian/vault/ | sort | uniq -c | sort -rn
```

---

## 🏆 THE BIG MISSION — COMPLETE!

You have now completed the entire AIOS Learning Adventure.

Here is the full workflow you now know:

```
┌────────────────────────────────────────────────────────────────┐
│              YOUR COMPLETE RESEARCH WORKFLOW                    │
│                                                                 │
│  1. FIND a document (article, notes, textbook chapter)          │
│         ↓                                                       │
│  2. ANALYSE with: aios-vault analyze <file>                     │
│         ↓                                                       │
│  3. NOTE the Key Topics the AI found                            │
│         ↓                                                       │
│  4. CREATE one vault note per topic (in obsidian/vault/)        │
│         ↓                                                       │
│  5. ADD [[links]] between related notes                         │
│         ↓                                                       │
│  6. INDEX with: aios-vault review                               │
│         ↓                                                       │
│  7. EXPLORE with: aios-vault insights                           │
│         ↓                                                       │
│  8. ASK questions using vault notes as context                  │
│         ↓                                                       │
│  9. SAVE new insights as new vault notes                        │
│         ↓                                                       │
│ 10. REPEAT for the next document — vault grows smarter!         │
└────────────────────────────────────────────────────────────────┘
```

---

## Your Journey So Far

| Tutorial | Skill Learned |
|---|---|
| T00 | Waking up the AI and checking it is healthy |
| T01 | Feeding documents to the AI for analysis |
| T02 | Creating structured vault notes with metadata |
| T03 | Linking notes to build a knowledge web |
| T04 | Finding what you know (keyword + meaning search) |
| T05 | Giving vault notes to the AI as context for better answers |
| T06 | Running the complete research-to-vault-to-AI workflow |

---

## What Can You Do Now?

You are no longer a beginner! Here are ideas for what to explore next:

| Idea | What to Try |
|---|---|
| **More topics** | Pick any school subject and run the full workflow |
| **Bigger documents** | Paste a whole chapter from a book into a `.txt` file |
| **Daily notes** | Each day write 3 facts you learned. Add them to the vault |
| **Grow your web** | Pick your biggest "empty link" and write that note |
| **Teach someone** | Walk a friend through T00 — teaching is the best way to learn |

---

> 🌟 **Remember:** Every note you add makes the AI smarter for you.  
> Every question you save makes your vault more valuable.  
> The vault is YOUR personalised AI brain — and only you can build it.

**Happy exploring! 🚀**
