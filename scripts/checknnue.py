import struct
import numpy as np
import sys

# Path to NNUE file should be provided via command line argument or input
NNUE_PATH = ""
OFFSET = 2048  
CP_SCALE = 15000.0  

def parse_fen(fen):
    pieces_map = {'p': 0, 'n': 1, 'b': 2, 'r': 3, 'q': 4, 'k': 5}
    parts = fen.split()
    rows = parts[0].split('/')
    white_pieces = [] 
    black_pieces = []
    w_ksq, b_ksq = 0, 0

    for r, row in enumerate(rows):
        rank = 7 - r
        file = 0
        for char in row:
            if char.isdigit():
                file += int(char)
            else:
                sq = rank * 8 + file
                p_type = pieces_map[char.lower()]
                if char.isupper():
                    white_pieces.append((p_type, sq))
                    if p_type == 5: w_ksq = sq
                else:
                    black_pieces.append((p_type, sq))
                    if p_type == 5: b_ksq = sq
                file += 1
    
    side_to_move = 0 if parts[1] == 'w' else 1
    return white_pieces, black_pieces, w_ksq, b_ksq, side_to_move

def get_indices(white_pieces, black_pieces, w_ksq, b_ksq, buckets):
    KB = [0,0,1,1,4,4,5,5, 0,0,1,1,4,4,5,5, 2,2,3,3,6,6,7,7, 2,2,3,3,6,6,7,7,
          2,2,3,3,6,6,7,7, 2,2,3,3,6,6,7,7, 0,0,1,1,4,4,5,5, 0,0,1,1,4,4,5,5]
    
    bw = KB[w_ksq] % buckets
    bb = KB[b_ksq ^ 56] % buckets 

    def get_halfkp_indices(p_list_own, p_list_opp, k_bucket, is_white_view):
        idx = []

        for p_type, sq in p_list_own:
            s = sq if is_white_view else sq ^ 56
            idx.append(768 * k_bucket + (p_type * 64) + s)

        for p_type, sq in p_list_opp:
            s = sq if is_white_view else sq ^ 56
            idx.append(768 * k_bucket + 384 + (p_type * 64) + s)
        return idx

    return get_halfkp_indices(white_pieces, black_pieces, bw, True), \
           get_halfkp_indices(black_pieces, white_pieces, bb, False)

def run_eval(h1, buckets, fen, nnue_path):
    try:
        w_p, b_p, w_ksq, b_ksq, stm = parse_fen(fen)
        with open(nnue_path, 'rb') as f:
            f.seek(OFFSET)
            b1 = np.frombuffer(f.read(h1 * 2), dtype=np.int16).astype(np.float32)
            w1 = np.frombuffer(f.read(buckets * 768 * h1 * 2), dtype=np.int16).reshape(-1, h1).astype(np.float32)
            b2 = np.frombuffer(f.read(32 * 2), dtype=np.int16).astype(np.float32)
            w2 = np.frombuffer(f.read(2 * h1 * 32 * 2), dtype=np.int16).reshape(32, 2*h1).astype(np.float32)
            b3 = np.frombuffer(f.read(32 * 2), dtype=np.int16).astype(np.float32)
            w3 = np.frombuffer(f.read(32 * 32 * 2), dtype=np.int16).reshape(32, 32).astype(np.float32)
            b4 = np.frombuffer(f.read(1 * 2), dtype=np.int16).astype(np.float32)
            w4 = np.frombuffer(f.read(32 * 1 * 2), dtype=np.int16).reshape(1, 32).astype(np.float32)

        idx_w, idx_b = get_indices(w_p, b_p, w_ksq, b_ksq, buckets)
        
        aw, ab = np.copy(b1), np.copy(b1)
        for i in idx_w: aw += w1[i]
        for i in idx_b: ab += w1[i]
        
        o1 = np.concatenate([np.clip(aw/127.0, 0, 1), np.clip(ab/127.0, 0, 1)])
        o2 = np.maximum(0, b2 + w2 @ o1)
        o3 = np.maximum(0, b3 + w3 @ o2)
        raw = (b4 + w4 @ o3)[0]
        
        score = raw if stm == 0 else -raw
        return score / CP_SCALE
    except Exception as e:
        return None

if __name__ == "__main__":
    print("\n--- NNUE Multi-Config Verifier ---")
    
    if len(sys.argv) > 1:
        NNUE_PATH = sys.argv[1]
    else:
        NNUE_PATH = input("Enter NNUE file path: ").strip()
        
    if not NNUE_PATH:
        print("NNUE file path is required.")
        sys.exit(1)

    test_fen = input("Enter FEN (or press Enter for Startpos): ").strip()
    if not test_fen: test_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

    print(f"\nEvaluating: {test_fen}")
    print(f"{'H1':<5} | {'Buck':<5} | {'Score (cp)':<12} | {'Symmetry'}")
    print("-" * 45)

    for h1 in [128, 256, 512]:
        for bks in [1, 8, 16]:
            score = run_eval(h1, bks, test_fen, NNUE_PATH)
            if score is not None:

                parts = test_fen.split()
                parts[1] = 'b' if parts[1] == 'w' else 'w'
                opp_fen = " ".join(parts)
                opp_score = run_eval(h1, bks, opp_fen, NNUE_PATH)
                
                sym = "OK" if abs(score + opp_score) < 0.01 else "BAD"
                status = "DEAD" if abs(score) < 1e-7 else f"{score:+.2f}"
                
                print(f"{h1:<5} | {bks:<5} | {status:<12} | {sym}")