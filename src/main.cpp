#include "master.h"
#include "logger.h"
#include "defs.h"
#include <iostream>
#include <algorithm>
#include <random>
#include <vector>
#include <assert.h>

int max_size = 1000000;
int threshold = 100;

int partition(int *array, int l, int r) {
    int x = array[r - 1];
    int cur = l - 1;
    for (int i = l; i < r - 1; i++) {
        if (array[i] < x) {
            cur++;
            std::swap(array[i], array[cur]);
        }
    }
    std::swap(array[cur + 1], array[r - 1]);
    return cur + 1;
}

void quick_sort(void *args) {
    Arg *arg = (Arg*)args;
    LOG_INFO("%p: %d %d", std::this_thread::get_id(), arg->left, arg->right);
    check_stack();
    if (arg->right - arg->left < threshold) {
        std::sort(arg->array + arg->left, arg->array + arg->right);
    } else {
        // int middle = (arg->right + arg->left) / 2;
        int middle = partition(arg->array, arg->left, arg->right);
        
        // now i see why we need gc
        Arg *left_arg = new Arg;
        Arg *right_arg = new Arg;
        WaitGroup *child_wg = new WaitGroup(1);
        left_arg->left = arg->left;
        left_arg->right = middle;
        right_arg->left = middle + 1;
        right_arg->right = arg->right;
        left_arg->array = arg->array;
        right_arg->array = arg->array;
        left_arg->wg = child_wg;
        right_arg->wg = nullptr;

        create(quick_sort, left_arg);
        // quick_sort(left_arg);

        LOG_INFO("%p: %d %d", std::this_thread::get_id(), arg->left, arg->right);
        // check_stack();

        quick_sort(right_arg);
        // create(quick_sort, right_arg);

        // a pitfall here, WaitGroup will busy waiting
        // maybe switch to another task would be greater?
        child_wg->wait();
        
        // do the 2-way merge
        // int *tmp = new int[arg->right - arg->left];
        // int left = arg->left, right = middle;
        // int cnt = 0;
        // while (left < middle && right < arg->right) {
        //     if (arg->array[left] < arg->array[right]) {
        //         tmp[cnt++] = arg->array[left++];
        //     } else {
        //         tmp[cnt++] = arg->array[right++];
        //     }
        // }
        // while (left < middle) {
        //     tmp[cnt++] = arg->array[left++];
        // }
        // while (right < arg->right) {
        //     tmp[cnt++] = arg->array[right++];
        // }
        // // copy back
        // for (int i = 0; i < cnt; i++) {
        //     arg->array[i + arg->left] = tmp[i];
        // }
        // assert(cnt == arg->right - arg->left);

        // delete[] tmp;
    }
    if (arg->wg) {
        arg->wg->done();
    }
}

int main() {
    initialize();
    int *a = new int[max_size];
    int *b = new int[max_size];
    for (int i = 0; i < max_size; i++) {
        a[i] = rand();
        b[i] = a[i];
    }
    WaitGroup wg(1);
    Arg *arg = new Arg;
    arg->left = 0;
    arg->right = max_size;
    arg->array = a;
    arg->wg = &wg;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    create(quick_sort, arg);
    wg.wait();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::cout << "TinyThread Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;
    std::cout << "TinyThread Time difference = " << std::chrono::duration_cast<std::chrono::nanoseconds> (end - begin).count() << "[ns]" << std::endl;

    std::chrono::steady_clock::time_point begin1 = std::chrono::steady_clock::now();
    std::sort(b, b + max_size);
    std::chrono::steady_clock::time_point end1 = std::chrono::steady_clock::now();

    std::cout << "std::sort Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end1 - begin1).count() << "[µs]" << std::endl;
    std::cout << "std::sort Time difference = " << std::chrono::duration_cast<std::chrono::nanoseconds> (end1 - begin1).count() << "[ns]" << std::endl;
    
    return 0;
}