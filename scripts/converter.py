import os
import glob
import struct
import io
import gzip
import chess
import chess.pgn
import chess.polyglot

def polyglot_move(move, board):
    """Convert a chess.Move to polyglot integer format."""
    to_file = move.to_square & 7
    to_rank = move.to_square >> 3
    from_file = move.from_square & 7
    from_rank = move.from_square >> 3
    
    # Castling moves in Polyglot are represented by king capturing its own rook
    if board.is_castling(move):
        if move.to_square == chess.G1:
            to_file, to_rank = 7, 0  # H1
        elif move.to_square == chess.C1:
            to_file, to_rank = 0, 0  # A1
        elif move.to_square == chess.G8:
            to_file, to_rank = 7, 7  # H8
        elif move.to_square == chess.C8:
            to_file, to_rank = 0, 7  # A8

    promotion = 0
    if move.promotion:
        if move.promotion == chess.KNIGHT: promotion = 1
        elif move.promotion == chess.BISHOP: promotion = 2
        elif move.promotion == chess.ROOK: promotion = 3
        elif move.promotion == chess.QUEEN: promotion = 4

    return (promotion << 12) | (from_rank << 9) | (from_file << 6) | (to_rank << 3) | to_file

def write_bin_file(entries_dict, bin_path):
    flat_entries = []
    for hash_val, moves in entries_dict.items():
        for move_val, weight in moves.items():
            flat_entries.append((hash_val, move_val, min(weight, 0xFFFF), 0))
            
    flat_entries.sort(key=lambda x: x[0])
    
    print(f"  Writing {len(flat_entries)} entries to {bin_path}...")
    with open(bin_path, "wb") as f:
        for entry in flat_entries:
            f.write(struct.pack(">QHHI", entry[0], entry[1], entry[2], entry[3]))

def process_game(game, entries, max_ply=40):
    board = game.board()
    for move in game.mainline_moves():
        if board.ply() >= max_ply:
            break
            
        hash_val = chess.polyglot.zobrist_hash(board)
        move_val = polyglot_move(move, board)
        
        if hash_val not in entries:
            entries[hash_val] = {}
        
        if move_val not in entries[hash_val]:
            entries[hash_val][move_val] = 0
            
        entries[hash_val][move_val] += 1
        board.push(move)

def process_file(filepath, out_dir):
    filename = os.path.basename(filepath)
    out_path = os.path.join(out_dir, filename.rsplit('.', 1)[0] + '.bin')
    if out_path.endswith('.pgn.bin'):
        out_path = out_path[:-8] + '.bin'
    
    entries = {}
    print(f"Processing {filename}...")
    
    if filename.endswith(".pgn"):
        with open(filepath, "r", encoding="utf-8") as f:
            while True:
                game = chess.pgn.read_game(f)
                if game is None:
                    break
                process_game(game, entries)
                
    elif filename.endswith(".pgn.gz"):
        with gzip.open(filepath, "rt", encoding="utf-8") as f:
            while True:
                game = chess.pgn.read_game(f)
                if game is None:
                    break
                process_game(game, entries)
                
    elif filename.endswith(".txt"):
        with open(filepath, "r", encoding="utf-8") as f:
            for line in f:
                parts = line.strip().split(';')
                if len(parts) >= 2:
                    move_seq = parts[1].strip()
                    game = chess.pgn.read_game(io.StringIO(move_seq))
                    if game:
                        process_game(game, entries)
                        
    else:
        print(f"  Skipping {filename} (Unsupported format, extract first if it's .7z)")
        return
        
    write_bin_file(entries, out_path)


if __name__ == "__main__":
    raw_dir = os.path.abspath("data/raw_books")
    base_bin_dir = os.path.abspath("data/books")
    
    # Process opening and endgame subfolders
    for subfolder in ["opening", "endgame"]:
        current_raw = os.path.join(raw_dir, subfolder)
        current_bin = os.path.join(base_bin_dir, subfolder)
        
        if not os.path.exists(current_raw):
            continue
            
        os.makedirs(current_bin, exist_ok=True)
        
        files = []
        for ext in ["*.txt", "*.pgn", "*.pgn.gz"]:
            files.extend(glob.glob(os.path.join(current_raw, ext)))
            
        if not files:
            print(f"No opening books found in {current_raw}")
        else:
            for f in files:
                process_file(f, current_bin)
                
    print("\nAll done!")
