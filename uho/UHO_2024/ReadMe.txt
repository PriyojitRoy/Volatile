
The Unbalanced Human Openings 2024 (UHO 2024)

All work done by Stefan Pohl (SPCC (www.sp-cc.de))

The idea of the UHO (Unbalanced Human Openings) openings is quite simple. The chess-community is a real conservative group. 
And I recognized, that many of them dont like openings, which were constructed (my Drawkiller openings for example) or changed 
manually (like my NBSC openings, where black is not allowed to castle short). They say, that is no "real chess". I do not agree 
with that personally, but I decided to try building openings-sets, which are working like NBSC (white has a clear advantage) 
and can (not must!) be (because white has a clear advantage at the beginning of the game) rescored with my Advanced Armageddon 
Scoring system for extremly high Elo-spreadings, which makes the rakings in a test/tournament much more reliable with a lower 
number of played games, but are complete unmodified and 100% human.

Whats new in UHO 2024:
The UHO 2024 opening sets are filtered out of the Megabase 2024 (by ChessBase). And the only filter, I am using, 
is the eval of KomodoDragon 3.3 in the endposition of each opening-line. And the UHO 2024 files are around +100% bigger than 
the UHO 2022 files, because the the Elo-limit was lowered to 2200 (instead of 2300 Elo in UHO 2022) (=both human players
had to have an Elo of >= 2200).

What did I do exactly? 

- sorted the games of the Megabase 2024 by Elo (using SCID)
- cutted all games after 6/8 moves (12/16 plies).
- removed all Chess960 games
- removed all lines with double endpositons, so each endposition is unique. And because the games are sorted by Elo of 
players, each opening line in the UHO-sets is played by the strongest players (if weaker players played the same opening
line, this game was deleted by pgn-extract, because pgn-extract always keeps the first appearance of a line in a pgn-file)
- removed all lines, were not both queens are still on board in the endposition.

After doing these work, there were 185983 6-moves deep opening-lines remaining and 532810 8-moves deep opening-lines
remaining (all with unique endpositions).
Finally, these opening-lines were evaluated (endpositions) with KomodoDragon 3.3 (10 seconds per endposition on a 8 core 
AMD Ryzen CPU) and kept all endpositions in an eval interval of [-1.99;+1.99] (this is the UHO 2024 RawData (can be found 
in the download, too)). That took around 80 days of computing...
The UHO 2024 RawData can be very useful for other purposes, because it contains all opening-lines from very good for black 
to very good for white. Feel free to filter these openings as you like...The eval (and search depth) is stored in the
Annotator-Tag of each line:
[Annotator "depth=29 eval=+114"]
You can use ChessBase (for example) for searching. Examples of searching:
"eval=-05" finds all lines in an eval-interval of [-0.50;-0.59]
"eval=+00" finds all lines in an eval-interval of [+0.00;+0.09]
"eval=-1" finds all lines in an eval-interval of [-1.00;-1.99]


So the Unbalanced Human Openings contain:

- 100% moves played by humans, only. Both players had at least 2200 Elo.
- opening lines played by the stronges players are at the beginning of the pgn/epd-files.
- no manually constructed openings (like my Drawkiller openings)
- no manually added moves to make castling impossible (like my NBSC openings)
- no selection of piece-patterns

There are 8 folders with different UHO 2024 openings-sets, with increasing advantage for white:
*** IMPORTANT: *** KomodoDragon 3.3 shows evals, which are around -0.25 lower, than KomodoDragon 2.6 does, 
which I used for evaluating the UHO 2022 openings. So, the Eval-intervals in the new UHO 2024 are lower:
UHO 2024 starts with Eval [+0.85;+0.94] instead of Eval [+1.10;+1.19] (UHO 2022). 

Eval [+0.85;+0.94] 6mvs: 6696 lines  8mvs: 19303 lines  8mvs_big: 37622 lines
Eval [+0.90;+0.99] 6mvs: 6292 lines  8mvs: 17145 lines  8mvs_big: 33559 lines
Eval [+0.95;+1.04] 6mvs: 5238 lines  8mvs: 14256 lines  8mvs_big: 30372 lines
Eval [+1.00;+1.09] 6mvs: 4740 lines  8mvs: 13227 lines  8mvs_big: 25916 lines
Eval [+1.05;+1.14] 6mvs: 4125 lines  8mvs: 11660 lines  8mvs_big: 22828 lines
Eval [+1.10;+1.19] 6mvs: 3460 lines  8mvs: 9601 lines   8mvs_big: 20600 lines
Eval [+1.15;+1.24] 6mvs: 3161 lines  8mvs: 8940 lines   8mvs_big: 17470 lines
Eval [+1.20;+1.29] 6mvs: 2717 lines  8mvs: 7869 lines   8mvs_big: 15464 lines

Each folder contains 3 UHO openings-sets: 6 moves, 8 moves and a bigger 8 moves file with a bigger eval interval of 
0.19 instead of 0.09.
As you can see, in UHO 2024 the Eval-stages "overlap" ([+0.85;+0.94] [+0.90;+0.99] for example)... the reason is,
that the draw-ratio can be adjusted better: Each higher eval-stage of UHO 2024 lowers the draw-ratio around -5%.

The idea is to adjust the draw-ratio of engine-tests and tournaments in a recommended interval of around 45%-60% by 
using a different UHO 2024 opening set. 
At the moment, it is recommended to firts the first UHO 2024 set with [+0.85;+0.94]. But if this set gives a too high
draw-ratio, because of a very drawish test-environment (Stockfish vs. another Stockfish for example), you can use a 
higher UHO 2024 set, to shrink the draw-ratio.
Same for the computerchess in the future, where faster hardware and stronger engines will lead to increasing
draw-ratio, too. Then using a higher UHO 2024 (more advantage for white) set can be useful.
But mention, that the draw-ratio should not get too low (below 45%), because this will lead to a lot of 1:1-pairs
(= one opening line is clearly won for white, when Engine A and Engine B plays white in a head-to-head, both Engines
win one game, using that opening-line (=1:1 points/pair)). If the draw-ratio in your engine-testings is below 45%, 
you should switch to a lower UHO 2024 set (=smaller advantage for white) otherwise you will shrink the Elo spreading 
of your test-results!!!

Mention, that the UHO 2024 files are getting smaller and smaller, the higher the Eval-interval is, because the games
in the Megabase getting rarer and rarer, giving white such a huge advantage after 6/8 moves.

The UHO openings give white a clear advantage, so these openings can rescored to gamepairs using my Gampairs Rescorer 
Tool (included in the download)


(C) 2024 Stefan Pohl (SPCC)(www.sp-cc.de)


Testresults:

Stockfish 16 vs. Torch 1, singlethread, 3min+1sec, 1000 games. Results from Stockfish 16 point of view.

For comparison:

Balanced openings:
SF_8moves_v3  :     1000 (+64,=916,-20),     Score: 52.2%, Elo: +15, Draws: 91.6%
Feobos-6m-v2.1:     1000 (+78,=911,-11),     Score: 53.4%, Elo: +24, Draws: 91.1%

UHO big file, used in Fishtest:
UHO_4060_v3   :     1000 (+375,=470,-155),   Score: 61.0%, Elo: +78, Draws: 47.0%


New UHO 2024:
6mvs_+085_+094:     1000 (+335,=569,- 96),   Score: 62.0%, Elo: +85, Draws: 56.9%
8mvs_+085_+094:     1000 (+347,=537,-116),   Score: 61.5%, Elo: +82, Draws: 53.7%
8mvs_big_+080_+099: 3000 (+1008,=1654,-338), Score: 61.2%, Elo: +80, Draws: 55.1%

6mvs_+090_+099:     1000 (+354,=525,-121),   Score: 61.6%, Elo: +83, Draws: 52.5%
8mvs_+090_+099:     1000 (+354,=502,-144),   Score: 60.5%, Elo: +75, Draws: 50.2%
8mvs_big_+085_+104: 3000 (+1097,=1453,-450), Score: 60.8%, Elo: +77, Draws: 48.4%
  
6mvs_+095_+104:     1000 (+380,=456,-164),   Score: 60.8%, Elo: +77, Draws: 45.6%
8mvs_+095_+104:     1000 (+415,=426,-159),   Score: 62.8%, Elo: +92, Draws: 42.6%
8mvs_big_+090_+109: 3000 (+1161,=1336,-503), Score: 61.0%, Elo: +78, Draws: 44.5%

6mvs_+100_+109:     1000 (+410,=403,-187),   Score: 61.1%, Elo: +79, Draws: 40.3%
8mvs_+100_+109:     1000 (+400,=404,-196),   Score: 60.2%, Elo: +73, Draws: 40.4%
8mvs_big_+095_+114: 3000 (+1222,=1205,-573), Score: 60.8%, Elo: +77, Draws: 40.2%

6mvs_+105_+114:     1000 (+425,=356,-219),   Score: 60.3%, Elo: +73, Draws: 35.6%
8mvs_+105_+114:     1000 (+430,=329,-241),   Score: 59.5%, Elo: +67, Draws: 32.9%
8mvs_big_+100_+119: 3000 (+1289,=1057,-654), Score: 60.6%, Elo: +75, Draws: 35.2%


Average draw-ratio of all 3 testruns of one UHO eval-level:
55.2%
      -4.8%
50.4%
      -6.2%
44.2%
      -3.9% 
40.3%

Means: around -5% less draws, with each higher UHO eval-level.


