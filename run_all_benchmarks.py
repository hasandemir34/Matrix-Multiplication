import os
import subprocess
import random

def generate_input(filename, n, m, p, q):
    with open(filename, 'w') as f:
        f.write(f"{n} {m}\n")
        for _ in range(n):
            f.write(" ".join(str(random.randint(0, 9)) for _ in range(m)) + "\n")
        f.write("\n")
        f.write(f"{p} {q}\n")
        for _ in range(p):
            f.write(" ".join(str(random.randint(0, 9)) for _ in range(q)) + "\n")

def main():
    sizes = [
       (50, 80, 80, 60),
        (120, 150, 150, 100),
        (200, 200, 200, 200),
        (300, 300, 300, 300),
    ]

    print("=== Matrix Multiplication Multi-Language Benchmark ===\n")
    
    # 1. Compile C++ and Java
    print("Compiling C++ and Java files...")
    exe_name = "main.exe" if os.name == 'nt' else "./main"
    subprocess.run(["g++", "-O3", "main.cpp", "-o", "main"], shell=(os.name == 'nt'))
    subprocess.run(["javac", "Main.java"])
    print("Compilation finished.\n")

    for i, (n, m, p, q) in enumerate(sizes):
        filename = f"benchmark_input_{i}.txt"
        print(f"--- Testing Size: {n}x{m} * {p}x{q} ---")
        generate_input(filename, n, m, p, q)
        
        # Run C++
        print(f"\n[C++] Execution:")
        result_cpp = subprocess.run([exe_name, filename], capture_output=True, text=True)
        print(result_cpp.stdout if result_cpp.returncode == 0 else result_cpp.stderr)

        # Run Java
        print(f"\n[Java] Execution:")
        result_java = subprocess.run(["java", "Main", filename], capture_output=True, text=True)
        print(result_java.stdout if result_java.returncode == 0 else result_java.stderr)

        # Run Python
        print(f"\n[Python] Execution:")
        result_py = subprocess.run(["python", "main.py", filename], capture_output=True, text=True)
        print(result_py.stdout if result_py.returncode == 0 else result_py.stderr)
        
        # Clean up
        if os.path.exists(filename):
            os.remove(filename)
        print("-" * 50)

if __name__ == "__main__":
    main()
