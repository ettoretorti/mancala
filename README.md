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
## Progress so Far
1. Game logic and board
2. Random Agent
3. User Agent
4. Minimax agent
5. Unit Tests 

## In Progress
1. Monte Carlo Agent
2. Hybrid Agent
3. Performance Testing
4. Precomputation of opening and closing tables

## Method
Our early game strategy consists of using an opening book for the first N <= ~10 moves precomputed ahead of time with extensive monte carlo simulations. Our end game strategy consists of using closing book precomputed for all positions with fewer than ~15 stones in play with exhaustive minimax. This will hopefully result in us gaining an advantageous position early on, and will guarantee optimal play towards the end.

Our midgame strategy will consist of two steps. Firstly, a shallow minimax search will be performed to discard any moves that are guaranteed to end in a loss. If any move guaranteeing a win is found, it will be returned immediately. We'll apply the usual optimizations involving alpha-beta pruning and move reordering based on simple heuristics. An estimate of the value of each move returned by the minimax will be stored.

Secondly, we shall start a monte carlo tree search based on UCT, with moves scored highly by the previous minimax search being given a higher initial weight.

Both these searches will resort to checking the closing book once the number of stones remaining on the board is small enough.

If at any point we determine that it is impossible for us to lose (our score - opponent score > stones remaining on board), we can just randomly select a valid move each turn (unless the final difference in score matters). The same goes for when we are guaranteed to lose, but that will never happen :)

## Distribution of work
We've decided to adopt a pair programming approach to the work. This means we'll be taking turns working on all the outstanding things.

## Possible Heuristics for minimax
1. Your Score - Opponent Score
2. # of stones captured by making the move
3. # of stones in your opponent's holes opposite your empty holes
4. Whether the move allows you to go again
5. Neural network trained on positions evaluated with monte carlo
