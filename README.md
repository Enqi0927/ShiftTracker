# Shift & Expense Tracker

A small C++ console app I built to help track my part-time work hours and pay.  
It records each shift into a CSV file, calculates total pay, and even gives a rough estimate of UK tax for fun.  

---

##  Why I made this
I work part-time while studying Computer Science, and I wanted a simple way to log my shifts and see how much I’d earned that week or month.  
Instead of doing it manually in Excel, I decided to build my own lightweight tracker using C++.

---

##  What it does
- Add a shift with date, hours, hourly rate, and notes  
- List all recorded shifts in order  
- Show total pay in recent days (e.g. last 7 days)  
- Summarise monthly income  
- Roughly estimate yearly UK tax (for demo purposes only)

Example:
tracker --add 2025-11-10 5 12.5 "Evening shift"
tracker --recent 7
tracker --summary


---

##  Key Features
- Reads and writes data to a CSV file (`data/shifts.csv`)
- Object-oriented structure: `Shift`, `Tracker`, and `FileStorage`
- Works cross-platform (Windows & macOS)
- Basic date/time handling and exception checking

---

##  What I learned
This project helped me practise:
- File I/O and CSV parsing  
- Object-Oriented Programming in C++  
- Exception handling and modular program design  
- Writing something genuinely useful for daily life  

---

##  Author
Created by **Enqi Weng**  
Second-year Computer Science student at City, University of London (2025)

