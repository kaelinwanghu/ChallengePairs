# ChallengePairs
For-fun project that gets a graph of alive people with Wikipedian articles and tries to find the longest shortest path (no cycles) among them. 
Also includes an option for tallying just a lot of longest shortest paths beyond a certain length.
Rewritten from the Rose-Hulman CSSE230 "GraphSurfing" project from Java into C++ for faster performance and better multithreading. The algorithm avoids brute-force (very impractical) and instead tries to find "popular" nodes to DFS from both forwards and backwards.
