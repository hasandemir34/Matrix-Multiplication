import java.io.*;
import java.util.*;

class MatrixParseError extends Exception {
    public MatrixParseError(String message) {
        super(message);
    }
    public MatrixParseError(String message, Throwable cause) {
        super(message, cause);
    }
}

class MatrixParser {
    public static class ParsedMatrix {
        public int[][] arrayMatrix;
        public ArrayList<ArrayList<Integer>> listMatrix;
        public int rows;
        public int cols;
    }

    private static boolean isBlank(String line) {
        return line == null || line.trim().isEmpty();
    }

    private static int[] parseDimensions(String line, String matrixName) throws MatrixParseError {
        String normalized = line.toLowerCase().replace("x", " ").replace("*", " ");
        String[] parts = normalized.trim().split("\\s+");
        if (parts.length != 2) {
            throw new MatrixParseError("Invalid dimension line for " + matrixName + ": '" + line + "'");
        }
        try {
            int rows = Integer.parseInt(parts[0]);
            int cols = Integer.parseInt(parts[1]);
            if (rows <= 0 || cols <= 0) {
                throw new MatrixParseError("Invalid dimensions for " + matrixName + ": " + rows + "x" + cols);
            }
            return new int[]{rows, cols};
        } catch (NumberFormatException e) {
            throw new MatrixParseError("Non-numeric dimensions in " + matrixName + ".", e);
        }
    }

    public static ParsedMatrix readOneMatrix(Scanner scanner, String matrixName) throws MatrixParseError {
        String line = "";
        while (scanner.hasNextLine()) {
            line = scanner.nextLine();
            if (!isBlank(line)) {
                break;
            }
        }
        
        if (isBlank(line)) {
            throw new MatrixParseError("Missing " + matrixName + " dimensions.");
        }

        int[] dims = parseDimensions(line, matrixName);
        int rows = dims[0];
        int cols = dims[1];

        ParsedMatrix pm = new ParsedMatrix();
        pm.rows = rows;
        pm.cols = cols;
        pm.arrayMatrix = new int[rows][cols];
        pm.listMatrix = new ArrayList<>();

        for (int i = 0; i < rows; i++) {
            if (!scanner.hasNextLine()) {
                throw new MatrixParseError("Missing elements in a row (" + matrixName + ", row " + (i + 1) + ")");
            }
            line = scanner.nextLine();
            if (isBlank(line)) {
                throw new MatrixParseError("Missing elements in a row (" + matrixName + ", row " + (i + 1) + ")");
            }
            
            String[] tokens = line.trim().split("\\s+");
            if (tokens.length < cols) {
                throw new MatrixParseError("Missing elements in a row (" + matrixName + ", row " + (i + 1) + ")");
            }
            if (tokens.length > cols) {
                throw new MatrixParseError("Extra elements in a row (" + matrixName + ", row " + (i + 1) + ")");
            }
            
            ArrayList<Integer> rowList = new ArrayList<>(cols);
            for (int j = 0; j < cols; j++) {
                try {
                    int val = Integer.parseInt(tokens[j]);
                    pm.arrayMatrix[i][j] = val;
                    rowList.add(val);
                } catch (NumberFormatException e) {
                    throw new MatrixParseError("Non-numeric values detected (" + matrixName + ", row " + (i + 1) + ")", e);
                }
            }
            pm.listMatrix.add(rowList);
        }
        return pm;
    }
}

class MatrixMultiplicator {
    private static int multiplyFunc(int a, int b) {
        return a * b;
    }

    // --- Array Implementations ---
    public static void multiplyArrayDirect(int[][] a, int[][] b, int[][] out) {
        int rows = a.length;
        int shared = a[0].length;
        int cols = b[0].length;
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                out[i][j] = 0;
            }
            for (int k = 0; k < shared; k++) {
                int aik = a[i][k];
                for (int j = 0; j < cols; j++) {
                    out[i][j] += aik * b[k][j];
                }
            }
        }
    }

    public static void multiplyArrayFunction(int[][] a, int[][] b, int[][] out) {
        int rows = a.length;
        int shared = a[0].length;
        int cols = b[0].length;
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                out[i][j] = 0;
            }
            for (int k = 0; k < shared; k++) {
                int aik = a[i][k];
                for (int j = 0; j < cols; j++) {
                    out[i][j] += multiplyFunc(aik, b[k][j]);
                }
            }
        }
    }

    // --- ArrayList Implementations ---
    public static void multiplyListDirect(ArrayList<ArrayList<Integer>> a, ArrayList<ArrayList<Integer>> b, ArrayList<ArrayList<Integer>> out) {
        int rows = a.size();
        int shared = a.get(0).size();
        int cols = b.get(0).size();
        
        for (int i = 0; i < rows; i++) {
            ArrayList<Integer> outRow = out.get(i);
            for (int j = 0; j < cols; j++) {
                outRow.set(j, 0);
            }
            ArrayList<Integer> aRow = a.get(i);
            for (int k = 0; k < shared; k++) {
                int aik = aRow.get(k);
                ArrayList<Integer> bkRow = b.get(k);
                for (int j = 0; j < cols; j++) {
                    outRow.set(j, outRow.get(j) + aik * bkRow.get(j));
                }
            }
        }
    }

    public static void multiplyListFunction(ArrayList<ArrayList<Integer>> a, ArrayList<ArrayList<Integer>> b, ArrayList<ArrayList<Integer>> out) {
        int rows = a.size();
        int shared = a.get(0).size();
        int cols = b.get(0).size();
        
        for (int i = 0; i < rows; i++) {
            ArrayList<Integer> outRow = out.get(i);
            for (int j = 0; j < cols; j++) {
                outRow.set(j, 0);
            }
            ArrayList<Integer> aRow = a.get(i);
            for (int k = 0; k < shared; k++) {
                int aik = aRow.get(k);
                ArrayList<Integer> bkRow = b.get(k);
                for (int j = 0; j < cols; j++) {
                    outRow.set(j, outRow.get(j) + multiplyFunc(aik, bkRow.get(j)));
                }
            }
        }
    }
}

class Benchmark {
    private static final int RUNS = 5;

    public static double runArrayBenchmark(int[][] a, int[][] b, boolean useFunction) {
        int rows = a.length;
        int cols = b[0].length;
        int[][] out = new int[rows][cols];
        
        long totalNanos = 0;
        for (int i = 0; i < RUNS; i++) {
            long start = System.nanoTime();
            if (useFunction) {
                MatrixMultiplicator.multiplyArrayFunction(a, b, out);
            } else {
                MatrixMultiplicator.multiplyArrayDirect(a, b, out);
            }
            long end = System.nanoTime();
            totalNanos += (end - start);
        }
        return (totalNanos / 1_000_000.0) / RUNS;
    }

    public static double runListBenchmark(ArrayList<ArrayList<Integer>> a, ArrayList<ArrayList<Integer>> b, boolean useFunction) {
        int rows = a.size();
        int cols = b.get(0).size();
        
        ArrayList<ArrayList<Integer>> out = new ArrayList<>(rows);
        for (int i = 0; i < rows; i++) {
            ArrayList<Integer> row = new ArrayList<>(cols);
            for (int j = 0; j < cols; j++) row.add(0);
            out.add(row);
        }

        long totalNanos = 0;
        for (int i = 0; i < RUNS; i++) {
            long start = System.nanoTime();
            if (useFunction) {
                MatrixMultiplicator.multiplyListFunction(a, b, out);
            } else {
                MatrixMultiplicator.multiplyListDirect(a, b, out);
            }
            long end = System.nanoTime();
            totalNanos += (end - start);
        }
        return (totalNanos / 1_000_000.0) / RUNS;
    }
}

public class Main {
    public static void printTableHeader() {
        System.out.printf("%-10s%-32s%-24s%-15s%n", "Language", "Implementation", "Size", "Avg. Time (ms)");
        for (int i = 0; i < 81; i++) System.out.print("-");
        System.out.println();
    }

    public static void printTableRow(String lang, String impl, String size, double avgMs) {
        System.out.printf("%-10s%-32s%-24s%-15.3f%n", lang, impl, size, avgMs);
    }

    public static void printMatrixIfSmall(int[][] matrix, String label) {
        int rows = matrix.length;
        int cols = (rows > 0) ? matrix[0].length : 0;
        if (rows >= 10 || cols >= 10) return;

        System.out.println("\n" + label + " (" + rows + "x" + cols + "):");
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                System.out.print(matrix[i][j] + (j + 1 == cols ? "" : " "));
            }
            System.out.println();
        }
    }

    public static void main(String[] args) {
        if (args.length < 1) {
            System.err.println("Usage: java Main <input_file>");
            System.exit(1);
        }

        String filename = args[0];
        try (Scanner scanner = new Scanner(new File(filename))) {
            MatrixParser.ParsedMatrix matrixA = MatrixParser.readOneMatrix(scanner, "Matrix A");
            MatrixParser.ParsedMatrix matrixB = MatrixParser.readOneMatrix(scanner, "Matrix B");

            if (matrixA.cols != matrixB.rows) {
                throw new MatrixParseError("Incompatible dimensions for multiplication: " + matrixA.cols + " != " + matrixB.rows);
            }

            String sizeText = matrixA.rows + "x" + matrixA.cols + " * " + matrixB.rows + "x" + matrixB.cols;

            double avgArrDirect = Benchmark.runArrayBenchmark(matrixA.arrayMatrix, matrixB.arrayMatrix, false);
            double avgArrFunction = Benchmark.runArrayBenchmark(matrixA.arrayMatrix, matrixB.arrayMatrix, true);
            double avgListDirect = Benchmark.runListBenchmark(matrixA.listMatrix, matrixB.listMatrix, false);
            double avgListFunction = Benchmark.runListBenchmark(matrixA.listMatrix, matrixB.listMatrix, true);

            printTableHeader();
            printTableRow("Java", "int[][] - Direct", sizeText, avgArrDirect);
            printTableRow("Java", "int[][] - Function", sizeText, avgArrFunction);
            printTableRow("Java", "ArrayList - Direct", sizeText, avgListDirect);
            printTableRow("Java", "ArrayList - Function", sizeText, avgListFunction);

            int[][] demoOut = new int[matrixA.rows][matrixB.cols];
            MatrixMultiplicator.multiplyArrayDirect(matrixA.arrayMatrix, matrixB.arrayMatrix, demoOut);
            printMatrixIfSmall(demoOut, "Result Matrix");

        } catch (FileNotFoundException e) {
            System.err.println("Input error: Cannot open input file: " + filename);
            System.exit(1);
        } catch (MatrixParseError e) {
            System.err.println("Input error: " + e.getMessage());
            System.exit(1);
        } catch (Exception e) {
            System.err.println("Unexpected error: " + e.getMessage());
            System.exit(1);
        }
    }
}
