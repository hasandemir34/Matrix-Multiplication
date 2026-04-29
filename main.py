import sys
import time
from typing import List, Tuple


class MatrixParseError(Exception):
    pass


def multiply(a: int, b: int) -> int:
    return a * b


def is_blank(line: str) -> bool:
    return line.strip() == ""


def parse_dimensions(line: str, matrix_name: str) -> Tuple[int, int]:
    normalized = line.replace("x", " ").replace("X", " ").replace("*", " ")
    parts = normalized.split()
    if len(parts) != 2:
        raise MatrixParseError(f"Invalid dimension line for {matrix_name}: '{line.strip()}'")

    try:
        rows = int(parts[0])
        cols = int(parts[1])
    except ValueError as exc:
        raise MatrixParseError(f"Non-numeric dimensions in {matrix_name}.") from exc

    if rows <= 0 or cols <= 0:
        raise MatrixParseError(f"Invalid dimensions for {matrix_name}: {rows}x{cols}")
    return rows, cols


def parse_row(line: str, expected_cols: int, row_index: int, matrix_name: str) -> List[int]:
    tokens = line.split()
    if len(tokens) < expected_cols:
        raise MatrixParseError(f"Missing elements in {matrix_name} row {row_index + 1}")
    if len(tokens) > expected_cols:
        raise MatrixParseError(f"Extra elements in {matrix_name} row {row_index + 1}")

    row: List[int] = []
    for token in tokens:
        try:
            value = int(token)
        except ValueError as exc:
            raise MatrixParseError(f"Non-numeric value in {matrix_name} row {row_index + 1}") from exc
        if value < 0 or value > 9:
            raise MatrixParseError(
                f"Value out of range [0,9] in {matrix_name} row {row_index + 1}"
            )
        row.append(value)
    return row


def read_one_matrix(lines: List[str], start_idx: int, matrix_name: str) -> Tuple[List[List[int]], int]:
    idx = start_idx
    while idx < len(lines) and is_blank(lines[idx]):
        idx += 1

    if idx >= len(lines):
        raise MatrixParseError(f"Missing {matrix_name} dimensions.")

    rows, cols = parse_dimensions(lines[idx], matrix_name)
    idx += 1

    matrix: List[List[int]] = []
    for row_i in range(rows):
        if idx >= len(lines):
            raise MatrixParseError(f"Missing elements in {matrix_name} row {row_i + 1}")
        if is_blank(lines[idx]):
            raise MatrixParseError(f"Missing elements in {matrix_name} row {row_i + 1}")
        matrix.append(parse_row(lines[idx], cols, row_i, matrix_name))
        idx += 1

    return matrix, idx


def multiply_direct(a: List[List[int]], b: List[List[int]], out: List[List[int]]) -> None:
    rows = len(a)
    shared = len(a[0])
    cols = len(b[0])
    for i in range(rows):
        for j in range(cols):
            out[i][j] = 0
        for k in range(shared):
            aik = a[i][k]
            for j in range(cols):
                out[i][j] += aik * b[k][j]


def multiply_function(a: List[List[int]], b: List[List[int]], out: List[List[int]]) -> None:
    rows = len(a)
    shared = len(a[0])
    cols = len(b[0])
    for i in range(rows):
        for j in range(cols):
            out[i][j] = 0
        for k in range(shared):
            aik = a[i][k]
            for j in range(cols):
                out[i][j] += multiply(aik, b[k][j])


def benchmark(a: List[List[int]], b: List[List[int]], use_function: bool, runs: int = 5) -> float:
    rows = len(a)
    cols = len(b[0])
    out = [[0 for _ in range(cols)] for _ in range(rows)]
    total_ms = 0.0

    for _ in range(runs):
        start = time.perf_counter()
        if use_function:
            multiply_function(a, b, out)
        else:
            multiply_direct(a, b, out)
        end = time.perf_counter()
        total_ms += (end - start) * 1000.0

    return total_ms / runs


def print_table_header() -> None:
    print(f"{'Language':<10}{'Implementation':<32}{'Size':<24}{'Avg. Time (ms)':<15}")
    print("-" * 81)


def print_table_row(language: str, implementation: str, size: str, avg_ms: float) -> None:
    print(f"{language:<10}{implementation:<32}{size:<24}{avg_ms:<15.3f}")


def print_matrix_if_small(matrix: List[List[int]], label: str) -> None:
    rows = len(matrix)
    cols = len(matrix[0]) if rows > 0 else 0
    if rows >= 10 or cols >= 10:
        return

    print(f"\n{label} ({rows}x{cols}):")
    for row in matrix:
        print(" ".join(str(value) for value in row))


def main() -> int:
    if len(sys.argv) < 2:
        print(f"Usage: python {sys.argv[0]} <input_file>", file=sys.stderr)
        return 1

    input_file = sys.argv[1]

    try:
        with open(input_file, "r", encoding="utf-8") as f:
            lines = f.readlines()

        matrix_a, idx = read_one_matrix(lines, 0, "Matrix A")
        matrix_b, _ = read_one_matrix(lines, idx, "Matrix B")

        n = len(matrix_a)
        m = len(matrix_a[0])
        p = len(matrix_b)
        q = len(matrix_b[0])

        if m != p:
            raise MatrixParseError(f"Incompatible dimensions for multiplication: {m} != {p}")

        size_text = f"{n}x{m} * {p}x{q}"

        avg_direct = benchmark(matrix_a, matrix_b, use_function=False, runs=5)
        avg_function = benchmark(matrix_a, matrix_b, use_function=True, runs=5)

        print_table_header()
        print_table_row("Python", "List of lists - Direct", size_text, avg_direct)
        print_table_row("Python", "List of lists - Function", size_text, avg_function)

        demo_out = [[0 for _ in range(q)] for _ in range(n)]
        multiply_direct(matrix_a, matrix_b, demo_out)
        print_matrix_if_small(demo_out, "Result Matrix")

    except FileNotFoundError:
        print(f"Input error: Cannot open input file: {input_file}", file=sys.stderr)
        return 1
    except MatrixParseError as exc:
        print(f"Input error: {exc}", file=sys.stderr)
        return 1
    except Exception as exc:  # Fallback for unexpected runtime errors.
        print(f"Unexpected error: {exc}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
