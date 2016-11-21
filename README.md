# mancala
Mancala bot for AI &amp; Games

## Building & Testing
```
git submodule init && git submodule update
mkdir build && cd build
cmake ..
make

./alltests
```
## EL Plan

#### Progress so Far
1. Game Board
2. Random Agent
3. User Agent
3. Unit Testing

#### In Progress
1. Mini-Max Agent
2. Parser for Protocol
3. Monte Carlo Agent

#### TO DO
1. Performance Testing
2. Training Data

#### Method
Our Agent will play itself for a very large number of games. We will use the results of those games and moves associated to create the opening table. The Closing table will be generated from brute for search.

The rest of the playable game will be done using a combination of Minimax, Monte Carlo Search, Alpha-Beta Pruning, Precomputed Tables and Heuristics.

#### Agent
1. Closing Table
  This will consist of us generating all Board containing at most N stones, where N is around ~20 and doing minimax all the way to the terminating nodes.
2. Opening Table
  Similar to Closing table. Minimax up to a certain depth then applying Monte Carlo to approximate the values

#### Heuristics
1. Your Score - Opponent Score
2. # of stones captured
3. # of Stones on your opponent side opposing empty holes
4. Play Randomly after guaranteeing win
5. Play Randomly after guaranteeing loss
