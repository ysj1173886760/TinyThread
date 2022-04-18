#include "master.h"
#include "logger.h"
#include "defs.h"
#include <iostream>
#include <algorithm>
#include <random>
#include <vector>
#include <assert.h>

void quick_sort(void *args) {
    Arg *arg = (Arg*)args;
    LOG_INFO("%p: %d %d", std::this_thread::get_id(), arg->left, arg->right);
    check_stack();
    if (arg->right - arg->left < 10) {
        std::sort(arg->array + arg->left, arg->array + arg->right);
    } else {
        int middle = (arg->right + arg->left) / 2;
        
        // now i see why we need gc
        Arg *left_arg = new Arg;
        Arg *right_arg = new Arg;
        WaitGroup *child_wg = new WaitGroup(1);
        left_arg->left = arg->left;
        left_arg->right = middle;
        right_arg->left = middle;
        right_arg->right = arg->right;
        left_arg->array = arg->array;
        right_arg->array = arg->array;
        left_arg->wg = child_wg;
        right_arg->wg = nullptr;

        create(quick_sort, left_arg);
        // quick_sort(left_arg);

        LOG_INFO("%p: %d %d", std::this_thread::get_id(), arg->left, arg->right);
        check_stack();

        quick_sort(right_arg);
        child_wg->wait();
        
        // do the 2-way merge
        int *tmp = new int[arg->right - arg->left];
        int left = arg->left, right = middle;
        int cnt = 0;
        while (left < middle && right < arg->right) {
            if (arg->array[left] < arg->array[right]) {
                tmp[cnt++] = arg->array[left++];
            } else {
                tmp[cnt++] = arg->array[right++];
            }
        }
        while (left < middle) {
            tmp[cnt++] = arg->array[left++];
        }
        while (right < arg->right) {
            tmp[cnt++] = arg->array[right++];
        }
        // copy back
        for (int i = 0; i < cnt; i++) {
            arg->array[i + arg->left] = tmp[i];
        }
        assert(cnt == arg->right - arg->left);

        delete[] tmp;
    }
    if (arg->wg) {
        arg->wg->done();
    }
}

int main() {
    initialize();
    int max_size = 100;
    int *a = new int[max_size];
    for (int i = 0; i < max_size; i++) {
        a[i] = rand() % 100;
    }
    WaitGroup wg(1);
    Arg *arg = new Arg;
    arg->left = 0;
    arg->right = max_size;
    arg->array = a;
    arg->wg = &wg;
    create(quick_sort, arg);
    wg.wait();
    for (int i = 0; i < max_size; i++) {
        std::cout << a[i] << " ";
    }
    return 0;
}