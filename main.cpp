#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include <chrono>
#include <iomanip>
#include <cctype>

using namespace std;
using namespace std::chrono;

class MatrixParseError : public runtime_error {
public:
    explicit MatrixParseError(const string& msg) : runtime_error(msg) {}
};

inline int multiply_func(int a, int b) {
    return a * b;
}

bool is_blank(const string& line) {
    for (char c : line) {
        if (!isspace(c)) return false;
    }
    return true;
}

void parse_dimensions(string line, const string& matrix_name, int& rows, int& cols) {
    for (char& c : line) {
        if (c == 'x' || c == 'X' || c == '*') c = ' ';
    }
    stringstream ss(line);
    string token;
    vector<string> parts;
    while (ss >> token) {
        parts.push_back(token);
    }
    if (parts.size() != 2) {
        throw MatrixParseError("Invalid dimension line for " + matrix_name + ": '" + line + "'");
    }
    try {
        rows = stoi(parts[0]);
        cols = stoi(parts[1]);
    } catch (...) {
        throw MatrixParseError("Non-numeric dimensions in " + matrix_name + ".");
    }
    if (rows <= 0 || cols <= 0) {
        throw MatrixParseError("Invalid dimensions for " + matrix_name + ": " + to_string(rows) + "x" + to_string(cols));
    }
}

vector<int> parse_row(const string& line, int expected_cols, int row_index, const string& matrix_name) {
    stringstream ss(line);
    string token;
    vector<int> row;
    while (ss >> token) {
        try {
            int value = stoi(token);
            if (value < 0 || value > 9) {
                // To mirror the python test logic if there is one
                throw MatrixParseError("Value out of range [0,9] in " + matrix_name + " row " + to_string(row_index + 1));
            }
            row.push_back(value);
        } catch (const MatrixParseError& e) {
            throw; // Rethrow out of range
        } catch (...) {
            throw MatrixParseError("Non-numeric values detected (" + matrix_name + ", row " + to_string(row_index + 1) + ")");
        }
    }
    if (row.size() < expected_cols) {
        throw MatrixParseError("Missing elements in a row (" + matrix_name + ", row " + to_string(row_index + 1) + ")");
    }
    if (row.size() > expected_cols) {
        throw MatrixParseError("Extra elements in a row (" + matrix_name + ", row " + to_string(row_index + 1) + ")");
    }
    return row;
}

void read_one_matrix(const vector<string>& lines, int& idx, const string& matrix_name, vector<vector<int>>& out_matrix) {
    while (idx < lines.size() && is_blank(lines[idx])) {
        idx++;
    }
    if (idx >= lines.size()) {
        throw MatrixParseError("Missing " + matrix_name + " dimensions.");
    }
    int rows, cols;
    parse_dimensions(lines[idx], matrix_name, rows, cols);
    idx++;

    out_matrix.clear();
    for (int i = 0; i < rows; ++i) {
        if (idx >= lines.size() || is_blank(lines[idx])) {
            throw MatrixParseError("Missing elements in a row (" + matrix_name + ", row " + to_string(i + 1) + ")");
        }
        out_matrix.push_back(parse_row(lines[idx], cols, i, matrix_name));
        idx++;
    }
}

// Vector Direct
void multiply_direct_vector(const vector<vector<int>>& a, const vector<vector<int>>& b, vector<vector<int>>& out) {
    int rows = a.size();
    int shared = a[0].size();
    int cols = b[0].size();
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            out[i][j] = 0;
            for (int k = 0; k < shared; ++k) {
                out[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

// Vector Function
void multiply_function_vector(const vector<vector<int>>& a, const vector<vector<int>>& b, vector<vector<int>>& out) {
    int rows = a.size();
    int shared = a[0].size();
    int cols = b[0].size();
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            out[i][j] = 0;
            for (int k = 0; k < shared; ++k) {
                out[i][j] += multiply_func(a[i][k], b[k][j]);
            }
        }
    }
}

// Dynamic Array Direct
void multiply_direct_array(int** a, int** b, int** out, int rows, int shared, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            out[i][j] = 0;
            for (int k = 0; k < shared; ++k) {
                out[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

// Dynamic Array Function
void multiply_function_array(int** a, int** b, int** out, int rows, int shared, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            out[i][j] = 0;
            for (int k = 0; k < shared; ++k) {
                out[i][j] += multiply_func(a[i][k], b[k][j]);
            }
        }
    }
}

double benchmark_vector(const vector<vector<int>>& a, const vector<vector<int>>& b, bool use_function, int runs = 5) {
    int rows = a.size();
    int cols = b[0].size();
    vector<vector<int>> out(rows, vector<int>(cols, 0));
    double total_ms = 0.0;

    for (int i = 0; i < runs; ++i) {
        auto start = high_resolution_clock::now();
        if (use_function) {
            multiply_function_vector(a, b, out);
        } else {
            multiply_direct_vector(a, b, out);
        }
        auto end = high_resolution_clock::now();
        total_ms += duration<double, milli>(end - start).count();
    }
    return total_ms / runs;
}

double benchmark_array(int** a, int** b, int rows, int shared, int cols, bool use_function, int runs = 5) {
    int** out = new int*[rows];
    for (int i = 0; i < rows; ++i) out[i] = new int[cols];

    double total_ms = 0.0;
    for (int r = 0; r < runs; ++r) {
        auto start = high_resolution_clock::now();
        if (use_function) {
            multiply_function_array(a, b, out, rows, shared, cols);
        } else {
            multiply_direct_array(a, b, out, rows, shared, cols);
        }
        auto end = high_resolution_clock::now();
        total_ms += duration<double, milli>(end - start).count();
    }

    for (int i = 0; i < rows; ++i) delete[] out[i];
    delete[] out;

    return total_ms / runs;
}

void print_table_header() {
    cout << left << setw(10) << "Language" 
         << setw(32) << "Implementation" 
         << setw(24) << "Size" 
         << "Avg. Time (ms)" << "\n";
    cout << string(81, '-') << "\n";
}

void print_table_row(const string& lang, const string& impl, const string& size, double avg_ms) {
    cout << left << setw(10) << lang 
         << setw(32) << impl 
         << setw(24) << size 
         << fixed << setprecision(3) << avg_ms << "\n";
}

void print_matrix_if_small(const vector<vector<int>>& matrix, const string& label) {
    int rows = matrix.size();
    int cols = rows > 0 ? matrix[0].size() : 0;
    if (rows >= 10 || cols >= 10) return;

    cout << "\n" << label << " (" << rows << "x" << cols << "):\n";
    for (const auto& row : matrix) {
        for (int i = 0; i < cols; ++i) {
            cout << row[i] << (i + 1 == cols ? "" : " ");
        }
        cout << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <input_file>\n";
        return 1;
    }

    string input_file = argv[1];
    vector<string> lines;
    ifstream file(input_file);
    if (!file) {
        cerr << "Input error: Cannot open input file: " << input_file << "\n";
        return 1;
    }

    string line;
    while (getline(file, line)) {
        lines.push_back(line);
    }
    file.close();

    try {
        vector<vector<int>> matrix_a, matrix_b;
        int idx = 0;
        read_one_matrix(lines, idx, "Matrix A", matrix_a);
        read_one_matrix(lines, idx, "Matrix B", matrix_b);

        int n = matrix_a.size();
        int m = matrix_a[0].size();
        int p = matrix_b.size();
        int q = matrix_b[0].size();

        if (m != p) {
            throw MatrixParseError("Incompatible dimensions for multiplication: " + to_string(m) + " != " + to_string(p));
        }

        string size_text = to_string(n) + "x" + to_string(m) + " * " + to_string(p) + "x" + to_string(q);

        // Convert to dynamic array
        int** arr_a = new int*[n];
        for (int i = 0; i < n; ++i) {
            arr_a[i] = new int[m];
            for (int j = 0; j < m; ++j) arr_a[i][j] = matrix_a[i][j];
        }

        int** arr_b = new int*[p];
        for (int i = 0; i < p; ++i) {
            arr_b[i] = new int[q];
            for (int j = 0; j < q; ++j) arr_b[i][j] = matrix_b[i][j];
        }

        double avg_arr_direct = benchmark_array(arr_a, arr_b, n, m, q, false);
        double avg_arr_function = benchmark_array(arr_a, arr_b, n, m, q, true);
        double avg_vec_direct = benchmark_vector(matrix_a, matrix_b, false);
        double avg_vec_function = benchmark_vector(matrix_a, matrix_b, true);

        print_table_header();
        print_table_row("C++", "Dynamic Array - Direct", size_text, avg_arr_direct);
        print_table_row("C++", "Dynamic Array - Function", size_text, avg_arr_function);
        print_table_row("C++", "Vector - Direct", size_text, avg_vec_direct);
        print_table_row("C++", "Vector - Function", size_text, avg_vec_function);

        vector<vector<int>> demo_out(n, vector<int>(q, 0));
        multiply_direct_vector(matrix_a, matrix_b, demo_out);
        print_matrix_if_small(demo_out, "Result Matrix");

        // Clean up
        for (int i = 0; i < n; ++i) delete[] arr_a[i];
        delete[] arr_a;
        for (int i = 0; i < p; ++i) delete[] arr_b[i];
        delete[] arr_b;

    } catch (const MatrixParseError& e) {
        cerr << "Input error: " << e.what() << "\n";
        return 1;
    } catch (const exception& e) {
        cerr << "Unexpected error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
