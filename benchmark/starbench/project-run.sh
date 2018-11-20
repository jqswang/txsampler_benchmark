#! /bin/bash

THREADS=2

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

#delete results file if exists
rm -f $DIR/benchmark_results.txt

echo "Begining benchmark. Results are saved in benchmark_results.txt"
echo "Number of threads: '$THREADS'"

export OMP_NUM_THREADS=$THREADS

#============================================================================
# Run K-means
#============================================================================

echo "Running K-means..."

echo "============== OpenMP ==============" >> $DIR/benchmark_results.txt

cd $DIR/kmeans/ompss
./kmeans -b -i $DIR/input/edge -n 2000 >> $DIR/benchmark_results.txt

echo "============= Pthreads =============" >> $DIR/benchmark_results.txt

cd ../pthread
./kmeans -b -i $DIR/input/edge -n 2000 -t $THREADS >> $DIR/benchmark_results.txt

echo "============ Sequential ============" >> $DIR/benchmark_results.txt

cd ../seq
./kmeans -b -i $DIR/input/edge -n 2000 >> $DIR/benchmark_results.txt

echo "Done."

#============================================================================
# Run ray-rot
#============================================================================

echo "Running ray-rot..."

echo "============== OpenMP ==============" >> $DIR/benchmark_results.txt

cd $DIR/ray-rot/ompss
./ray-rot $DIR/input/sphfract /dev/null 50 1920 1080 1 >> $DIR/benchmark_results.txt

echo "============= Pthreads =============" >> $DIR/benchmark_results.txt

cd ../pthread
./ray-rot $DIR/input/sphfract /dev/null 50 1920 1080 1 $THREADS 0 >> $DIR/benchmark_results.txt

echo "============ Sequential ============" >> $DIR/benchmark_results.txt

cd ../seq
./ray-rot $DIR/input/sphfract /dev/null 50 1920 1080 1 >> $DIR/benchmark_results.txt

echo "Done."


#============================================================================
# Run rot-cc
#============================================================================

echo "Running rot-cc..."

echo "============== OpenMP ==============" >> $DIR/benchmark_results.txt

cd $DIR/rot-cc/ompss
./rot-cc $DIR/input/Berlin_Botanischer-Garten_HB_02.ppm /dev/null 50 >> $DIR/benchmark_results.txt

echo "============= Pthreads =============" >> $DIR/benchmark_results.txt

cd ../pthread
./rot-cc $DIR/input/Berlin_Botanischer-Garten_HB_02.ppm /dev/null 50 $THREADS 0 >> $DIR/benchmark_results.txt

echo "============ Sequential ============" >> $DIR/benchmark_results.txt

cd ../seq
./rot-cc $DIR/input/Berlin_Botanischer-Garten_HB_02.ppm /dev/null 50 /dev/null >> $DIR/benchmark_results.txt

echo "Done."


#============================================================================
# Run rotate
#============================================================================

echo "Running rotate..."

echo "============== OpenMP ==============" >> $DIR/benchmark_results.txt

cd $DIR/rotate/ompss
./rot $DIR/input/Berlin_Botanischer-Garten_HB_02.ppm /dev/null 50 >> $DIR/benchmark_results.txt

echo "============= Pthreads =============" >> $DIR/benchmark_results.txt

cd ../pthread
./rot $DIR/input/Berlin_Botanischer-Garten_HB_02.ppm /dev/null 50 $THREADS 0 >> $DIR/benchmark_results.txt

echo "============ Sequential ============" >> $DIR/benchmark_results.txt

cd ../seq
./rot $DIR/input/Berlin_Botanischer-Garten_HB_02.ppm /dev/null 50  >> $DIR/benchmark_results.txt

echo "Done."

#============================================================================
# Run c-ray
#============================================================================

echo "Running c-ray..."

echo "============== OpenMP ==============" >> $DIR/benchmark_results.txt

cd $DIR/c-ray/omp
./c-ray-mt -i $DIR/input/sphfract -o /dev/null -s 1920x1080 -r 2 >> $DIR/benchmark_results.txt

echo "============= Pthreads =============" >> $DIR/benchmark_results.txt

cd ../pthread
./c-ray-mt -i $DIR/input/sphfract -o /dev/null -s 1920x1080 -r 2 -t $THREADS >> $DIR/benchmark_results.txt

echo "============ Sequential ============" >> $DIR/benchmark_results.txt

cd ../seq
./c-ray-mt -i $DIR/input/sphfract -o /dev/null -s 1920x1080 -r 2  >> $DIR/benchmark_results.txt

echo "Done."

#============================================================================
# Run md5
#============================================================================

echo "Running md5..."

echo "============== OpenMP ==============" >> $DIR/benchmark_results.txt

cd $DIR/md5/ompss
./md5 -i 7 -c 10 -t $THREADS >> $DIR/benchmark_results.txt

echo "============= Pthreads =============" >> $DIR/benchmark_results.txt

cd ../pthread
./md5 -i 7 -c 10 -t $THREADS -p 0 >> $DIR/benchmark_results.txt

echo "============ Sequential ============" >> $DIR/benchmark_results.txt

cd ../seq
./md5 -i 7 -c 10  >> $DIR/benchmark_results.txt

echo "Done."

#============================================================================
# Run rgbyuv
#============================================================================

echo "Running rgbyuv..."

echo "============== OpenMP ==============" >> $DIR/benchmark_results.txt

cd $DIR/rgbyuv/ompss
./rgbyuv -i $DIR/input/Berlin_Botanischer-Garten_HB_02.ppm -c 10 -t $THREADS  >> $DIR/benchmark_results.txt

echo "============= Pthreads =============" >> $DIR/benchmark_results.txt

cd ../pthread
./rgbyuv -i $DIR/input/Berlin_Botanischer-Garten_HB_02.ppm -c 10 -t $THREADS -p 0 >> $DIR/benchmark_results.txt

echo "============ Sequential ============" >> $DIR/benchmark_results.txt

cd ../seq
./rgbyuv -i $DIR/input/Berlin_Botanischer-Garten_HB_02.ppm -c 10  >> $DIR/benchmark_results.txt

echo "Done."

#============================================================================
# Run streamcluster
#============================================================================

echo "Running streamcluster..."

echo "============== OpenMP ==============" >> $DIR/benchmark_results.txt

cd $DIR/streamcluster/ompss
./streamcluster 10 20 128 200000 200000 5000 none output.txt $THREADS  >> $DIR/benchmark_results.txt

echo "============= Pthreads =============" >> $DIR/benchmark_results.txt

cd ../pthread
./streamcluster 10 20 128 200000 200000 5000 none output.txt $THREADS 0 >> $DIR/benchmark_results.txt

echo "============ Sequential ============" >> $DIR/benchmark_results.txt

cd ../seq
./streamcluster 10 20 128 200000 200000 5000 none output.txt  >> $DIR/benchmark_results.txt

echo "Done."

#============================================================================
echo "Benchmark completed."
