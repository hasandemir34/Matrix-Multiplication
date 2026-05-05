import random

def generate_large_input(filename, n, m, p, q):
    with open(filename, 'w') as f:
        # Matrix A boyutları
        f.write(f"{n} {m}\n")
        for _ in range(n):
            row = " ".join(str(random.randint(0, 9)) for _ in range(m))
            f.write(row + "\n")
        
        f.write("\n") # Matrisler arası boş satır
        
        # Matrix B boyutları
        f.write(f"{p} {q}\n")
        for _ in range(p):
            row = " ".join(str(random.randint(0, 9)) for _ in range(q))
            f.write(row + "\n")

# 500x500 boyutlarında test dosyası oluşturalım
generate_large_input("large_input.txt", 100, 100, 100, 100)
print("large_input.txt oluşturuldu!")