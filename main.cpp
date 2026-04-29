#include <chrono>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

class MatrixParseException : public runtime_error {
public:
    explicit MatrixParseException(const string& message) : runtime_error(message) {}
};

struct RawMatrix {
    int rows = 0;
    int cols = 0;
    int** data = nullptr;
};

void deallocateRawMatrix(RawMatrix& matrix);

struct RawMatrixOwner {
    RawMatrix matrix;
    ~RawMatrixOwner() { deallocateRawMatrix(matrix); }
};

void deallocateRawMatrix(RawMatrix& matrix) {
    if (matrix.data == nullptr) {
        return;
    }
    for (int i = 0; i < matrix.rows; ++i) {
        delete[] matrix.data[i];
    }
    delete[] matrix.data;
    matrix.data = nullptr;
    matrix.rows = 0;
    matrix.cols = 0;
}

RawMatrix allocateRawMatrix(int rows, int cols) {
    RawMatrix matrix;
    matrix.rows = rows;
    matrix.cols = cols;
    matrix.data = new int*[rows];
    for (int i = 0; i < rows; ++i) {
        matrix.data[i] = new int[cols];
        for (int j = 0; j < cols; ++j) {
            matrix.data[i][j] = 0;
        }
    }
    return matrix;
}

bool isBlankLine(const string& line) {
    for (char c : line) {
        if (!isspace(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    return true;
}

pair<int, int> parseDimensions(const string& line, const string& matrixName) {
    string normalized = line;
    for (char& c : normalized) {
        if (c == 'x' || c == 'X' || c == '*') {
            c = ' ';
        }
    }

    istringstream iss(normalized);
    int rows = 0;
    int cols = 0;
    string extra;
    if (!(iss >> rows >> cols) || (iss >> extra) || rows <= 0 || cols <= 0) {
        throw MatrixParseException("Invalid dimension line for " + matrixName + ": \"" + line + "\"");
    }
    return {rows, cols};
}

vector<int> parseMatrixRow(const string& line, int expectedCols, int rowIndex, const string& matrixName) {
    istringstream iss(line);
    vector<string> tokens;
    string token;
    while (iss >> token) {
        tokens.push_back(token);
    }

    if (static_cast<int>(tokens.size()) < expectedCols) {
        throw MatrixParseException("Missing elements in " + matrixName + " row " + to_string(rowIndex + 1));
    }
    if (static_cast<int>(tokens.size()) > expectedCols) {
        throw MatrixParseException("Extra elements in " + matrixName + " row " + to_string(rowIndex + 1));
    }

    vector<int> row(expectedCols);
    for (int j = 0; j < expectedCols; ++j) {
        size_t pos = 0;
        try {
            row[j] = stoi(tokens[j], &pos);
        } catch (const exception&) {
            throw MatrixParseException("Non-numeric value in " + matrixName + " row " + to_string(rowIndex + 1));
        }
        if (pos != tokens[j].size()) {
            throw MatrixParseException("Non-numeric value in " + matrixName + " row " + to_string(rowIndex + 1));
        }
        if (row[j] < 0 || row[j] > 9) {
            throw MatrixParseException(
                "Value out of range [0,9] in " + matrixName + " row " + to_string(rowIndex + 1)
            );
        }
    }
    return row;
}

vector<vector<int>> readOneMatrix(ifstream& input, const string& matrixName) {
    string line;

    while (getline(input, line)) {
        if (!isBlankLine(line)) {
            break;
        }
    }

    if (!input || isBlankLine(line)) {
        throw MatrixParseException("Missing " + matrixName + " dimensions.");
    }

    pair<int, int> dims = parseDimensions(line, matrixName);
    int rows = dims.first;
    int cols = dims.second;

    vector<vector<int>> matrix(rows, vector<int>(cols));
    for (int i = 0; i < rows; ++i) {
        if (!getline(input, line)) {
            throw MatrixParseException("Missing elements in " + matrixName + " row " + to_string(i + 1));
        }
        vector<int> row = parseMatrixRow(line, cols, i, matrixName);
        matrix[i] = row;
    }

    return matrix;
}

RawMatrix toRawMatrix(const vector<vector<int>>& source) {
    int rows = static_cast<int>(source.size());
    int cols = rows > 0 ? static_cast<int>(source[0].size()) : 0;
    RawMatrix matrix = allocateRawMatrix(rows, cols);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix.data[i][j] = source[i][j];
        }
    }
    return matrix;
}

int multiply(int a, int b) {
    return a * b;
}

void multiplyRawDirect(const RawMatrix& a, const RawMatrix& b, RawMatrix& result) {
    for (int i = 0; i < a.rows; ++i) {
        for (int j = 0; j < b.cols; ++j) {
            result.data[i][j] = 0;
        }
        for (int k = 0; k < a.cols; ++k) {
            const int aik = a.data[i][k];
            for (int j = 0; j < b.cols; ++j) {
                result.data[i][j] += aik * b.data[k][j];
            }
        }
    }
}

void multiplyRawFunction(const RawMatrix& a, const RawMatrix& b, RawMatrix& result) {
    for (int i = 0; i < a.rows; ++i) {
        for (int j = 0; j < b.cols; ++j) {
            result.data[i][j] = 0;
        }
        for (int k = 0; k < a.cols; ++k) {
            const int aik = a.data[i][k];
            for (int j = 0; j < b.cols; ++j) {
                result.data[i][j] += multiply(aik, b.data[k][j]);
            }
        }
    }
}

void multiplyVectorDirect(
    const vector<vector<int>>& a,
    const vector<vector<int>>& b,
    vector<vector<int>>& result
) {
    int rows = static_cast<int>(a.size());
    int shared = static_cast<int>(a[0].size());
    int cols = static_cast<int>(b[0].size());
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            result[i][j] = 0;
        }
        for (int k = 0; k < shared; ++k) {
            const int aik = a[i][k];
            for (int j = 0; j < cols; ++j) {
                result[i][j] += aik * b[k][j];
            }
        }
    }
}

void multiplyVectorFunction(
    const vector<vector<int>>& a,
    const vector<vector<int>>& b,
    vector<vector<int>>& result
) {
    int rows = static_cast<int>(a.size());
    int shared = static_cast<int>(a[0].size());
    int cols = static_cast<int>(b[0].size());
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            result[i][j] = 0;
        }
        for (int k = 0; k < shared; ++k) {
            const int aik = a[i][k];
            for (int j = 0; j < cols; ++j) {
                result[i][j] += multiply(aik, b[k][j]);
            }
        }
    }
}

double benchmarkRaw(const RawMatrix& a, const RawMatrix& b, bool useFunctionCall) {
    const int runs = 5;
    double totalMs = 0.0;
    RawMatrix result = allocateRawMatrix(a.rows, b.cols);

    for (int i = 0; i < runs; ++i) {
        auto start = chrono::high_resolution_clock::now();
        if (useFunctionCall) {
            multiplyRawFunction(a, b, result);
        } else {
            multiplyRawDirect(a, b, result);
        }
        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double, milli> elapsed = end - start;
        totalMs += elapsed.count();
    }
    deallocateRawMatrix(result);

    return totalMs / runs;
}

double benchmarkVector(const vector<vector<int>>& a, const vector<vector<int>>& b, bool useFunctionCall) {
    const int runs = 5;
    double totalMs = 0.0;
    vector<vector<int>> result(a.size(), vector<int>(b[0].size(), 0));

    for (int i = 0; i < runs; ++i) {
        auto start = chrono::high_resolution_clock::now();
        if (useFunctionCall) {
            multiplyVectorFunction(a, b, result);
        } else {
            multiplyVectorDirect(a, b, result);
        }
        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double, milli> elapsed = end - start;
        totalMs += elapsed.count();
    }

    return totalMs / runs;
}

void printTableHeader() {
    cout << left
         << setw(10) << "Language"
         << setw(32) << "Implementation"
         << setw(24) << "Size"
         << setw(15) << "Avg. Time (ms)" << '\n';
    cout << string(81, '-') << '\n';
}

void printTableRow(const string& language, const string& implementation, const string& size, double avgMs) {
    cout << left
         << setw(10) << language
         << setw(32) << implementation
         << setw(24) << size
         << setw(15) << fixed << setprecision(3) << avgMs << '\n';
}

void printMatrixIfSmall(const vector<vector<int>>& matrix, const string& label) {
    const int rows = static_cast<int>(matrix.size());
    const int cols = rows > 0 ? static_cast<int>(matrix[0].size()) : 0;
    if (rows >= 10 || cols >= 10) {
        return;
    }

    cout << "\n" << label << " (" << rows << "x" << cols << "):\n";
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            cout << matrix[i][j] << (j + 1 == cols ? '\n' : ' ');
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <input_file>\n";
        return 1;
    }

    try {
        ifstream input(argv[1]);
        if (!input.is_open()) {
            throw MatrixParseException("Cannot open input file: " + string(argv[1]));
        }

        vector<vector<int>> matrixA = readOneMatrix(input, "Matrix A");
        vector<vector<int>> matrixB = readOneMatrix(input, "Matrix B");

        int n = static_cast<int>(matrixA.size());
        int m = static_cast<int>(matrixA[0].size());
        int p = static_cast<int>(matrixB.size());
        int q = static_cast<int>(matrixB[0].size());

        if (m != p) {
            throw MatrixParseException("Incompatible dimensions for multiplication: " + to_string(m) + " != " + to_string(p));
        }

        RawMatrixOwner rawA{toRawMatrix(matrixA)};
        RawMatrixOwner rawB{toRawMatrix(matrixB)};

        string sizeText = to_string(n) + "x" + to_string(m) + " * " + to_string(p) + "x" + to_string(q);

        printTableHeader();
        printTableRow("C++", "Dynamic array - Direct", sizeText, benchmarkRaw(rawA.matrix, rawB.matrix, false));
        printTableRow("C++", "Dynamic array - Function", sizeText, benchmarkRaw(rawA.matrix, rawB.matrix, true));
        printTableRow("C++", "Vector - Direct", sizeText, benchmarkVector(matrixA, matrixB, false));
        printTableRow("C++", "Vector - Function", sizeText, benchmarkVector(matrixA, matrixB, true));

        vector<vector<int>> demoResult(n, vector<int>(q, 0));
        multiplyVectorDirect(matrixA, matrixB, demoResult);
        printMatrixIfSmall(demoResult, "Result Matrix");
    } catch (const MatrixParseException& e) {
        cerr << "Input error: " << e.what() << '\n';
        return 1;
    } catch (const exception& e) {
        cerr << "Unexpected error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
