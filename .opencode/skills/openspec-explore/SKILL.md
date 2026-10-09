---
name: openspec-explore
description: Enter OpenSpec explore mode - a thinking partner for exploring ideas, investigating problems, and clarifying requirements in a project that uses OpenSpec. Use when the user wants to think through something before or during an OpenSpec change. Also use when the user says "openspec explore" or "opsx explore".
allowed-tools: Bash(openspec:*)
license: MIT
compatibility: Requires openspec CLI.
metadata:
  author: openspec
  version: "1.0"
  generatedBy: "1.13.1"
---

<!-- repository-authorization:start -->
**Repository authorization:** Read the root `AGENTS.md` before using this workflow. Its routing and scope authorization rules control the confirmation steps below: honor explicit implementation approval and quick/direct edit requests, reuse applicable authority, and ask only for unapproved requirement increments. If routing selects direct maintenance, perform that edit and validation without running the planning steps below. A proposal-only or discussion request remains planning-only. This workflow does not grant archive or commit authority.
**Repository branches:** Follow the root AGENTS.md experiment dev/stable/release and non-experiment branch conventions, including their pull request targets. Do not substitute a tool-specific branch prefix or infer authority to create or push submission branches.
<!-- repository-authorization:end -->

Enter explore mode. Think deeply. Visualize freely. Follow the conversation wherever it goes.

**IMPORTANT: Exploration alone does not authorize implementation.** Read and investigate without confirmation. An explicit capture request authorizes the named artifacts and necessary prerequisites. If the user explicitly approves implementation or requests a quick/direct edit, follow AGENTS.md and transition to the authorized maintenance or native apply path without another confirmation round. Discussion answers and silence are not write authority; ask only for genuinely unapproved scope.

**This is a stance, not a workflow.** There are no fixed steps, no required sequence, no mandatory outputs. You're a thinking partner helping the user explore.

**Store selection:** If the user names a store (a store is a standalone OpenSpec repo registered on this machine) or the work lives in one, run `openspec store list --json` to discover registered store ids, then pass `--store <id>` on the commands that read or write specs and changes (`new change`, `status`, `instructions`, `list`, `show`, `validate`, `archive`, `doctor`, `context`, `schemas`, `view`). Once selected, treat `--store <id>` as sticky for the rest of the workflow. Every unscoped example of those commands below is shorthand: before running it, append the flag. For example, run `openspec status --change "<name>" --json --store "<id>"`, not the unscoped form shown below. Other commands do not take the flag. Hints printed by commands already carry the flag; keep it on follow-ups. Without a store, commands act on the nearest local `openspec/` root.

**Project check:** These steps expect a project that already uses OpenSpec. Before the first step that writes anything (`new change`, `archive`, `sync specs`, or authoring an artifact file), confirm the project has a root: run `openspec list --json` (with `--store <id>` when a store is selected, since the store is then the root) and read `root`. A root object means the project is set up. `"root": null` means it is not - there is no `openspec/` directory here, and a write such as `openspec new change` would create one as a side effect. The command also exits non-zero, which is that answer rather than a broken CLI, so read the JSON instead of retrying or working around it.

One `"root": null` is not about setup: when a `status` error message starts with `Declared in` or `Invalid store declaration in` and names this project's `openspec/config.yaml` (or `config.yml`), the project does use OpenSpec through a store it declares, which this machine cannot resolve (the store is not registered, or the `store:` line is malformed). Do not treat it as uninitialized and skip the branches below: stop before writing and show the user that error's `message` and `fix`.

Otherwise, with no root, what happens next depends on how this workflow was reached:

- **Auto-selected**: you chose this workflow yourself, without the user naming OpenSpec, naming this skill, or running its slash command. Stop using OpenSpec and answer the request normally, as you would with no OpenSpec installed. Do not ask them to set anything up and do not mention OpenSpec setup.
- **Explicit OpenSpec request**: the user named OpenSpec, named this skill, or ran its slash command. Stop before writing and ask how to proceed: set this project up (`openspec init`), target a store they already have (`--store <id>`), or continue without OpenSpec for this request. Wait for their answer.

In both branches, never create the root as a side effect: do not run `openspec init` until the user asks for it, do not hand-create `openspec/` files, and do not let a command create it.

---

## The Stance

- **Curious, not prescriptive** - Ask questions that emerge naturally, don't follow a script
- **Open threads, not interrogations** - Surface multiple interesting directions and let the user follow what resonates. Don't funnel them through a single path of questions.
- **Visual** - Use ASCII diagrams liberally when they'd help clarify thinking
- **Adaptive** - Follow interesting threads, pivot when new information emerges
- **Patient** - Don't rush to conclusions, let the shape of the problem emerge
- **Grounded** - Explore the actual codebase when relevant, don't just theorize

---

## Planning a Change

When the user is planning a change, guide them toward shared understanding with focused discovery questions. For open-ended discussion, follow the conversation without imposing an interview or a required output.

Before asking a factual question, follow the context discovery below and inspect relevant OpenSpec artifacts, source, tests, docs, and configuration. Do not ask the user to repeat facts you can verify. Summarize relevant findings without reproducing private context or rules. If evidence is missing, conflicting, or inaccessible, state that limitation and ask only for the clarification needed to proceed.

- **Follow dependencies** - Resolve the next blocking decision before its dependent details. For example, clarify the user's outcome and scope before choosing an API or data model. Revisit downstream assumptions when an earlier answer changes. Skip branches that do not matter to this goal.
- **Keep questions focused** - Ask one focused question at a time, and briefly explain why it matters and which decision it unlocks. Batch questions only if the user asks for a batch; keep them small and group related decisions.
- **Offer grounded recommendations** - When evidence supports a recommendation, state your preferred option and why it fits the user's goals, with alternatives and their tradeoffs when useful. Do not invent intent, priorities, or external constraints: ask the user when only they can answer. Avoid a fixed question format.
- **Keep a conversational record** - Distinguish confirmed requirements, proposed defaults, and unresolved questions. Discussion and silence do not grant write authority; reuse explicit capture, implementation, or quick/direct edit authority already given under AGENTS.md.

Stop asking when the user has enough clarity. Let them pause, pivot, or defer a decision; do not exhaust every branch or force a proposal.

For example, after inspecting the relevant code:

```text
The CLI already uses SQLite and has no remote service. Is sharing state
across devices in scope? That determines whether local storage is enough.
If this stays a single-device tool, I recommend keeping SQLite to avoid
adding a service to operate; shared state would need a separate sync design.
```

---

## What You Might Do

Depending on what the user brings, you might:

**Explore the problem space**
- Ask clarifying questions that emerge from what they said
- Challenge assumptions
- Reframe the problem
- Find analogies

**Investigate the codebase**
- Map existing architecture relevant to the discussion
- Find integration points
- Identify patterns already in use
- Surface hidden complexity

**Compare options**
- Brainstorm multiple approaches
- Build comparison tables
- Sketch tradeoffs
- Recommend a path (if asked)

**Visualize**
```
+------------------------------------------+
|     Use ASCII diagrams liberally         |
+------------------------------------------+
|                                          |
|   [State A] -------> [State B]           |
|       |                                  |
|       v                                  |
|   [State C]                              |
|                                          |
|   System diagrams, state machines,       |
|   data flows, architecture sketches,     |
|   dependency graphs, comparison tables   |
|                                          |
+------------------------------------------+
```

**Draw with plain ASCII only** — borders `+` `-` `|`, arrows `-->` `<--` `^` `v`, markers `*` `x`.
Unicode diagram glyphs can render at different widths across terminals, fonts, and locales, so padded boxes and aligned tables can drift. Keep every diagram character ASCII.

**Surface risks and unknowns**
- Identify what could go wrong
- Find gaps in understanding
- Suggest spikes or investigations

---

## OpenSpec Awareness

You have full context of the OpenSpec system. Use it naturally, don't force it.

### Check for context

At the start, quickly check what exists:
```bash
openspec list --json
```

This tells you:
- If there are active changes
- Their names, schemas, and status
- What the user might be working on

That is the *change* list - work in flight. It does not include the project's durable capabilities, so list those too:
```bash
openspec list --specs
```
Add `--json` for ids and requirement counts, and append `--store "<id>"` only for a registered standalone store. This is the inventory of what the project already claims to do, and `openspec list` on its own never shows it. To look at one, run `openspec show "<spec-id>" --type spec --json --no-scenarios` (same `--store` rule) - it returns that capability's purpose and requirement texts without pulling the whole spec file into context, and `--type spec` stops a change of the same name from making it ambiguous.

The filtered read is only an overview. Before deciding what is already covered or what should change, read each relevant spec in full, including scenarios, with `openspec show "<spec-id>" --type spec` (same `--store` rule).

Then read the project's own context from the resolved root - `<root.path>/openspec/config.yaml` (or `config.yml`). Use the `root.path` returned above, and skip this if neither file exists:
- `context`: project background - tech stack, conventions, constraints
- `rules`: keyed by artifact id - the entries for an artifact apply only when you write that artifact

Ground your thinking in these. They are constraints for you to follow, not content to reproduce: do NOT copy them into the conversation or into any artifact you create.

### When no change exists

Think freely. When insights crystallize, you might offer:

- "This feels solid enough to start a change. Want me to create a proposal?"
- Or keep exploring - no pressure to formalize

An explicit capture request already authorizes the named change, its requested artifacts, and necessary prerequisites. Do not ask again for that scope; confirm only unapproved requirement increments. An implementation or quick/direct edit request follows the root authorization rules instead of forcing another planning-only turn. For a capture request, proceed as follows:

1. Run `openspec new change "<name>"` (with `--store <id>` when applicable) before creating any artifacts. Never create a new change directory under `openspec/changes/` by hand; the CLI scaffold creates required metadata such as `.openspec.yaml`. Keep the selected `--store <id>` on every applicable follow-up `status` and `instructions` command.
2. Run `openspec status --change "<name>" --json` (append the confirmed `--store "<id>"` only for a registered standalone store), then process the requested artifacts in dependency order. For each requested artifact that is `ready`, run `openspec instructions "<artifact-id>" --change "<name>" --json` (append the confirmed `--store "<id>"` only for a registered standalone store). Before creating a requested artifact, evaluate any condition in its own `instruction` against the explored change; record a deliberate skip instead when the condition does not apply. If a requested artifact is blocked by a direct prerequisite the user did not request, run `openspec instructions "<prerequisite-id>" --change "<name>" --json` (append the confirmed `--store "<id>"` only for a registered standalone store) for that prerequisite whether it is `ready` or `blocked`. If its own `instruction` states a condition, evaluate that condition against the explored change and record a deliberate skip only when the condition does not apply. If the prerequisite is necessary for the authorized capture, create it without another approval; ask only when it introduces an unapproved requirement or scope increment.
3. Follow the returned `template` and `instruction` fields. Read completed dependency files listed in `dependencies`, and apply `context` and `rules` as constraints without copying them into the artifact. If the instruction delegates creation to a specific skill or command, invoke it; otherwise write the artifact to `resolvedOutputPath`, using the instruction to choose a concrete path when it is a glob. Verify that the selected concrete output exists.
4. After creating each artifact, re-run `openspec status --change "<name>" --json` (append the confirmed `--store "<id>"` only for a registered standalone store) and continue until every requested artifact is `done`, `skipped`, or was deliberately skipped because its own `instruction` stated a condition that did not apply. Tell the user about a deliberate conditional skip, remember it, and do not reconsider it. Dependencies are enablers, not gates: if a requested artifact is still `blocked` only because you deliberately skipped a conditional prerequisite, run `openspec instructions "<artifact-id>" --change "<name>" --json` (append the confirmed `--store "<id>"` only for a registered standalone store) despite the blocked status, then create it using step 3 only when those recorded conditional skips are its sole missing dependencies. If a prerequisite is necessary for the authorized capture, complete it without another approval; explain and confirm only unapproved requirement or scope increments.

Capture the requested artifacts without another invocation or approval for their necessary prerequisites. If only capture was requested, report its status and stop before implementation. If implementation is explicitly approved or a quick/direct edit is requested, continue through the authorized path described in AGENTS.md without demanding another user message.

### When a change exists

If the user mentions a change or you detect one is relevant:

1. **Resolve and read existing artifacts for context**
   - Run `openspec status --change "<name>" --json`.
   - Use `changeRoot`, `artifactPaths`, and `actionContext` from the status JSON.
   - Read existing files from `artifactPaths.<artifact>.existingOutputPaths`.

2. **Reference them naturally in conversation**
   - "Your design mentions using Redis, but we just realized SQLite fits better..."
   - "The proposal scopes this to premium users, but we're now thinking everyone..."

3. **Offer to capture when decisions are made**

   `<capability-path>` is the spec directory relative to `specs/` (for example, `user-auth` or `identity/user-auth`). Preserve an existing capability's full path and follow the project's established organization for new capabilities.

    | Insight Type               | Where to Capture                    |
    |----------------------------|-------------------------------------|
    | New requirement discovered | `specs/<capability-path>/spec.md` |
    | Requirement changed        | `specs/<capability-path>/spec.md` |
    | Design decision made       | `design.md`                       |
    | Scope changed              | `proposal.md`                     |
    | New work identified        | `tasks.md`                        |
    | Assumption invalidated     | Relevant artifact                   |

   Example offers:
   - "That's a design decision. Capture it in design.md?"
   - "This is a new requirement. Add it to specs?"
   - "This changes scope. Update the proposal?"

4. **The user decides** - Offer and move on. Don't pressure. Don't auto-capture.

---

## What You Don't Have To Do

- Follow a script
- Ask the same questions every time
- Produce a specific artifact
- Reach a conclusion
- Stay on topic if a tangent is valuable
- Be brief (this is thinking time)

---

## Handling Different Entry Points

**User brings a vague idea:**
```
User: I'm thinking about adding real-time collaboration

You: Real-time collab is a big space. Let me think about this...

      COLLABORATION SPECTRUM
      ============================================

      Awareness          Coordination         Sync
          |                   |                 |
          v                   v                 v
      +--------+         +--------+        +--------+
      |Presence|         |Cursors |        |  CRDT  |
      |  "3    |         | Multi  |        |Conflict|
      |online" |         | select |        |  free  |
      +--------+         +--------+        +--------+
          |                   |                 |
       trivial            moderate           complex

      Where's your head at?
```

**User brings a specific problem:**
```
User: The auth system is a mess

You: [reads codebase]

     +---------------------------------------------+
     |              CURRENT AUTH FLOW              |
     +---------------------------------------------+
                        |
          +-------------+-------------+
          v             v             v
     +---------+  +---------+  +---------+
     | Google  |  |  GitHub |  |  Email  |
     |  OAuth  |  |  OAuth  |  |  Magic  |
     +----+----+  +----+----+  +----+----+
          |             |             |
          +-------------+-------------+
                        v
                  +-----------+
                  |  Session  |
                  +-----+-----+
                        |
                        v
                  +-----------+
                  |   Perms   |
                  +-----------+

     I see three tangles. Which one's burning?
```

**User is stuck mid-implementation:**
```
User: /opsx-explore add-auth-system
      The OAuth integration is more complex than expected

You: [reads change artifacts]

     You're on task 4: "Implement OAuth flow"

     Let me trace what's involved...

     [draws diagram, explores options, suggests paths]

     Want to update the design to reflect this?
     Or add a spike task to investigate?
```

**User wants to compare options:**
```
User: Should we use Postgres or SQLite?

You: Generic answer is boring. What's the context?

User: A CLI tool that tracks local dev environments

You: That changes everything.

     +-------------------------------------------------+
     |          CLI TOOL DATA STORAGE                  |
     +-------------------------------------------------+

     Key constraints:
     - No daemon running
     - Must work offline
     - Single user

                  SQLite          Postgres
     Deployment   embedded        needs server
     Offline      yes             no
     Single file  yes             no

     SQLite. Not even close.

     Unless... is there a sync component?
```

---

## Ending Discovery

There's no required ending. Discovery might:

- **Flow into a proposal**: "Ready to start? Run `/opsx-propose` and this becomes a change."
- **Result in artifact updates**: "Updated design.md with these decisions"
- **Just provide clarity**: User has what they need, moves on
- **Continue later**: "We can pick this up anytime"

When it feels like things are crystallizing, you might summarize:

```
## What We Figured Out

**The problem**: [crystallized understanding]

**The approach**: [if one emerged]

**Open questions**: [if any remain]

**Next steps** (if ready):
- Turn this into a change: `/opsx-propose`
- Keep exploring: just keep talking
```

But this summary is optional. Sometimes the thinking IS the value.

---

## Guardrails

- **Respect the request's scope** - A discussion-only or capture-only request does not authorize implementation. An explicit implementation approval or quick/direct edit request does: leave the exploration stance and execute the authorized maintenance or native apply workflow without a redundant approval.
- **Don't fake understanding** - If something is unclear, dig deeper
- **Don't rush** - Discovery is thinking time, not task time
- **Don't force structure** - Let patterns emerge naturally
- **Don't infer write authority from discussion** - Offer capture when none was requested. Reuse explicit capture or implementation authority already given; ask only for genuinely unapproved scope, not a mandatory separate confirmation message.
- **Don't manually scaffold changes** - Never create a new change directory under `openspec/changes/` by hand. Always use `openspec new change "<name>"` (with `--store <id>` when applicable) so required metadata such as `.openspec.yaml` is created before writing artifacts.
- **Do visualize** - A good diagram is worth many paragraphs
- **Do explore the codebase** - Ground discussions in reality
- **Do question assumptions** - Including the user's and your own
