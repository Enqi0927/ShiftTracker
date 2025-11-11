# Shift & Expense Tracker (C++17)

## Why

As a City, University of London student working part‑time at two cafés in south‑east London, I kept losing track of my hours and pay. Scribbled notes went missing and spreadsheets felt overkill. I wanted a tool that was offline, simple and worked on any lab PC without installing extra software. This project is that tool.

## What

This command‑line utility lets you record shifts, list them, filter by date ranges or hourly rates, and summarise your income. Data is stored in a CSV file under `data/shifts.csv` so you can version it or open it elsewhere.

### Commands

- `tracker --help` – show usage instructions.
- `tracker --list` – list all recorded shifts, sorted by date.
- `tracker --add <date> <hours> <rate> [note]` – add a shift. The date must be `yyyy-mm-dd`.
- `tracker --recent N` – list shifts in the last `N` days and show their total pay.
- `tracker --monthly` – print total pay per calendar month (yyyy‑mm).
- `tracker --summary` – show total shifts, gross pay, a rough PAYE tax estimate, and the number of big shifts (£100+).
- `tracker --rate>=X` – list shifts with hourlyRate ≥ `X`.

## Design decisions

- **CSV over database:** This project is intentionally minimal. A plain CSV file requires no dependencies or setup and can be version controlled. The storage layer is behind an interface (`IStorage`) so you can swap to SQLite or JSON later.
- **UTC dates:** The tool stores dates in `yyyy-mm-dd` and converts them to `time_t` using UTC. This avoids daylight saving surprises when filtering by recent days.
- **Simplified PAYE:** The tax calculation is a demonstration only. It uses the UK personal allowance (£12,570) and basic/higher rate thresholds as of 2025 and scales quarterly income to a yearly estimate. Do not rely on it for tax filings.
- **Single file:** Keeping the implementation in one `.cpp` file makes it easier to share and review. In a larger project you would split domain, storage and CLI into separate modules.

## Known limitations

- Notes containing commas are not escaped in the CSV. If your notes include commas you should implement proper quoting in `Shift::toCSV()` and `fromCSV()`.
- Dates are only minimally validated. The code does not reject invalid dates like `2025-13-40`.
- The PAYE estimate does not handle higher‑rate bands beyond 40% or other UK tax deductions.
- Error messages are printed to `stderr` but the CLI does not currently return distinct exit codes for each error type.

## Build

On Windows with MinGW or MSVC:

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic src\main.cpp -o tracker.exe
```

On Linux or macOS:

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic src/main.cpp -o tracker
```

The program reads and writes `data/shifts.csv` relative to the current working directory. You can run it from the project root or from the `src` directory.

## Example

After adding a few shifts:

```
2025-10-01,5.5,12.5,Evening @ Bluewater
2025-10-03,4,13.0,Lunch @ Farringdon
2025-10-05,6,12.0,Weekend shift
```

Running `tracker --monthly` outputs:

```
2025-10,246.5
```

## Future work

- Escape quotes and commas properly in CSV.
- Strictly validate dates and provide clearer suggestions on mistakes.
- Add more filters, e.g. `--between 2025-10-01 2025-10-31`.
- Persist data in SQLite with a schema and migrate existing CSV.
- Integrate with Catch2 or another unit testing framework.