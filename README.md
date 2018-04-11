# Stream anonymization
The goal of the project is to anonymize streamed data in a way that the resulting dataset approximates a specified k-anonymity and t-closeness.

# Current status
The program currently reads the provided csv data file and runs several, currently hardcoded rules on it, saving the results to output.csv.

# Sources
- irishcensus100m.csv: https://github.com/ucd-pel/COCOA.git
- CMakeLists.txt: Inspiration from https://github.com/cpplibv/libv.git
