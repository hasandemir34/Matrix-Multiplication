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

    print("Compiling Java file...\n")
    subprocess.run(["javac", "Main.java"])

    print("Generating matrices and running Java benchmarks...\n")
    
    for i, (n, m, p, q) in enumerate(sizes):
        filename = f"test_input_{i}.txt"
        print(f"\n--- Testing Size: {n}x{m} * {p}x{q} ---")
        generate_input(filename, n, m, p, q)
        
        # Run the Java Main class
        result = subprocess.run(["java", "Main", filename], capture_output=True, text=True)
        if result.returncode == 0:
            print(result.stdout)
        else:
            print(f"Error running benchmark:\n{result.stderr}")
        
        # Clean up
        if os.path.exists(filename):
            os.remove(filename)

if __name__ == "__main__":
    main()
