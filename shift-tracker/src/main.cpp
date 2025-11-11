// ----------------------------------------------------
// Shift & Expense Tracker
// Author: Enqi Weng
// Institution: City, University of London
// Date: November 2025
// Description: Personal project for tracking part-time work hours and pay.
// ----------------------------------------------------

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <map>
#include <memory>
#include <stdexcept>
#include <ctime>
using namespace std;

// ---------- Domain ----------
struct Shift {
    string date;      // ISO: yyyy-mm-dd
    double hours{};
    double hourlyRate{};
    string note;

    static Shift fromCSV(const string& line, size_t lineno) {
        vector<string> parts;
        string cur;
        for (char c: line) {
            if (c == ',') { parts.push_back(cur); cur.clear(); }
            else cur.push_back(c);
        }
        parts.push_back(cur);
        if (parts.size() < 3)
            throw runtime_error("Bad CSV at line " + to_string(lineno));

        Shift s;
        s.date = parts[0];
        try {
            s.hours = stod(parts[1]);
            s.hourlyRate = stod(parts[2]);
        } catch (...) {
            throw runtime_error("Bad number at line " + to_string(lineno));
        }
        if (parts.size() >= 4) s.note = parts[3];
        return s;
    }

    string toCSV() const {
        ostringstream oss;
        oss << date << "," << hours << "," << hourlyRate << "," << note;
        return oss.str();
    }

    double pay() const { return hours * hourlyRate; }
};

// ---------- Storage ----------
struct IStorage {
    virtual ~IStorage() = default;
    virtual vector<Shift> load() = 0;
    virtual void save(const vector<Shift>&) = 0;
};

struct FileStorage : IStorage {
    string path;
    explicit FileStorage(string p): path(move(p)) {}
    vector<Shift> load() override {
        vector<Shift> v;
        ifstream in(path);
        if (!in) return v; // empty if not exists
        string line; size_t ln = 0;
        while (getline(in, line)) {
            ++ln;
            if (line.empty()) continue;
            v.push_back(Shift::fromCSV(line, ln));
        }
        return v;
    }
    void save(const vector<Shift>& v) override {
        ofstream out(path);
        if (!out) throw runtime_error("Cannot open file for write: " + path);
        for (auto& s: v) out << s.toCSV() << "\n";
    }
};

// ---------- Utilities ----------
static tm parseISO(const string& ymd) {
    tm t{};
    if (ymd.size() >= 10) {
        t.tm_year = stoi(ymd.substr(0,4)) - 1900;
        t.tm_mon  = stoi(ymd.substr(5,2)) - 1;
        t.tm_mday = stoi(ymd.substr(8,2));
    }
    t.tm_hour = 0; t.tm_min = 0; t.tm_sec = 0;
    return t;
}

// cross-platform UTC time_t
static time_t utc_timegm(tm t) {
#if defined(_WIN32)
    return _mkgmtime(&t);   // Windows: convert tm (UTC) to time_t
#else
    return timegm(&t);      // POSIX/GNU
#endif
}

// // just estimate based on UK tax bands, roughly

static double estimateTaxYearly(double gross) {
    const double personalAllowance = 12570.0;
    const double basicBandLimit = 50270.0;
    if (gross <= personalAllowance) return 0.0;
    double taxable = gross - personalAllowance;
    double tax = 0.0;
    double basicBand = max(0.0, min(taxable, basicBandLimit - personalAllowance));
    tax += basicBand * 0.2;
    double higherBand = max(0.0, taxable - basicBand);
    tax += higherBand * 0.4;
    return tax;
}

// ---------- Tracker ----------
class Tracker {
    vector<Shift> shifts;
    unique_ptr<IStorage> storage;
public:
    explicit Tracker(unique_ptr<IStorage> s): storage(move(s)) { shifts = storage->load(); }

    void add(const Shift& s) { shifts.push_back(s); storage->save(shifts); }

    vector<Shift> listAllSorted() const {
        vector<Shift> v = shifts;
        sort(v.begin(), v.end(), [](const Shift& a, const Shift& b){ return a.date < b.date; });
        return v;
    }

    vector<Shift> filterRecentDays(int days) const {
        time_t now = time(nullptr);
        time_t cutoff = now - static_cast<time_t>(days)*24*3600;
        vector<Shift> out;
        copy_if(shifts.begin(), shifts.end(), back_inserter(out), [&](const Shift& s){
            return utc_timegm(parseISO(s.date)) >= cutoff;
        });
        return out;
    }

    double totalPay(const vector<Shift>& v) const {
        return accumulate(v.begin(), v.end(), 0.0, [](double acc, const Shift& s){ return acc + s.pay(); });
    }

    map<string, double> monthlyTotals() const {
        map<string,double> m;
        for (auto& s: shifts) m[s.date.substr(0,7)] += s.pay(); // yyyy-mm
        return m;
    }

    size_t countHighPay(double threshold) const {
        return count_if(shifts.begin(), shifts.end(), [&](const Shift& s){ return s.pay() >= threshold; });
    }
};

// ---------- CLI ----------
static void printHelp() {
    cout <<
R"(Shift & Expense Tracker
Usage:
  tracker --help
  tracker --list
  tracker --recent 7
  tracker --add 2025-10-01 5.5 12.5 "Lunch shift"
  tracker --monthly
  tracker --summary
Files:
  data/shifts.csv
)";
}

int main(int argc, char** argv) try {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<string> args(argv+1, argv+argc);
    if (args.empty() || args[0] == "--help") { printHelp(); return 0; }

    Tracker tracker(make_unique<FileStorage>("data/shifts.csv"));

    if (args[0] == "--add" && args.size() >= 4) {
        Shift s;
        s.date = args[1];
        s.hours = stod(args[2]);
        s.hourlyRate = stod(args[3]);
        if (args.size() >= 5) s.note = args[4];
        tracker.add(s);
        cout << "Added: " << s.toCSV() << "\n";
        return 0;
    }

    if (args[0] == "--list") {
        for (auto& s: tracker.listAllSorted())
            cout << s.toCSV() << "\n";
        return 0;
    }

    if (args[0] == "--recent" && args.size() >= 2) {
        int days = stoi(args[1]);
        auto v = tracker.filterRecentDays(days);
        for (auto& s: v) cout << s.toCSV() << "\n";
        cout << "Total pay in last " << days << " days: " << tracker.totalPay(v) << "\n";
        return 0;
    }

    if (args[0] == "--monthly") {
        for (auto& kv : tracker.monthlyTotals())
            cout << kv.first << "," << kv.second << "\n";
        return 0;
    }

    if (args[0] == "--summary") {
        auto all = tracker.listAllSorted();
        double gross = tracker.totalPay(all);
        cout << "Shifts: " << all.size() << "\n";
        cout << "Gross (pretax): " << gross << "\n";
        cout << "Very rough PAYE estimate (yearly scaled): "
             << estimateTaxYearly(gross * 52.0/4.0) / (52.0/4.0)
             << " (for demo)\n";
        cout << ">=£100 shifts: " << tracker.countHighPay(100.0) << "\n";
        return 0;
    }

    cerr << "Unknown command. Use --help.\n";
    return 1;

} catch (const exception& e) {
    cerr << "Error: " << e.what() << "\n";
    return 2;
}
