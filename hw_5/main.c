#include <stdio.h>
#include "bvMemory.h"

int main() {
    printf("Testing first fit\n");
    
    printf("Test first fit in the base case of just adding a bunch of stuff consecutively\n");
    // firstFit
    void* data = bvMalloc(10, 0);
    void* data2 = bvMalloc(10, 0);
    void* data3 = bvMalloc(10, 0);
    void* data4 = bvMalloc(10, 0);
    void* size1 = bvMalloc(1,0);
    printNodes(head);
    printf("---------------------\n");
   
    bvFree(size1);
    printf("Test where the head is not the first address in the heap\n");
    // firstFit
    bvFree(data);
    void* d = bvMalloc(10, 0);
    printNodes(head);
    printf("---------------------\n");

    printf("Test where the open space is in between nodes\n");
    // firstFit
    bvFree(data2);
    bvFree(data3);
    void* data5 = bvMalloc(20,0);
    printNodes(head);
    printf("---------------------\n");


    printf("Test freeing everything and then adding stuff back\n");
    bvFree(d);
    bvFree(data5);
    bvFree(data4);
    printNodes(head);
    printf("This should be empty\n");
    printf("---------------------\n");
    printf("Test adding something of size 1000\n");
    void* data6 = bvMalloc(1000,0);
    printNodes(head);
    printf("---------------------\n");
    
    printf("Test mallocing something over the size available\n");
    void* data7 = bvMalloc(1, 0);
    printf("%p\n", data7);
    bvFree(data6);
    printf("---------------------\n");
    printf("---------------------\n");
    printf("---------------------\n");
    printf("---------------------\n");
    
    printf("Testing best fit\n");
    printf("Testing basic adding in a row\n");
    // bestFit
    void* data10 = bvMalloc(10, 1);
    void* data11 = bvMalloc(10, 1);
    void* data12 = bvMalloc(10, 1);
    void* data13 = bvMalloc(10, 1);
    printNodes(head);
    printf("---------------------\n");
   
    printf("Testing freeing the head\n");
    // bestFit
    bvFree(data10);
    printNodes(head);
    printf("---------------------\n");

    printf("Testing adding something into its place\n");
    // bestFit
    void* data14 = bvMalloc(10,1);
    printNodes(head);
    printf("---------------------\n");
    
    printf("Testing freeing stuff in the middle and adding something in its place\n");
    bvFree(data11);
    bvFree(data12);
    void* data15 = bvMalloc(20, 1);
    printNodes(head);
    printf("---------------------\n");
    
    printf("Testing adding something that is too big\n");
    void* data16 = bvMalloc(100000, 1);
    printf("%p\n", data16);
    printf("---------------------\n");
    
    printf("Freeing everything left\n"); 
    bvFree(data13);
    bvFree(data14);
    bvFree(data15);
    printf("This should be empty\n");
    printNodes(head);
    printf("---------------------\n");
    
    printf("Testing when the better free spot is not the first\n");
    printf("Adding a bunch fo data frist\n");
    void* data17 = bvMalloc(10, 1);
    void* data18 = bvMalloc(20, 1);
    void* data19 = bvMalloc(10, 1);
    void* data20 = bvMalloc(8, 1);
    void* data21 = bvMalloc(10, 1);
    printNodes(head);
    printf("---------------------\n");
    printf("Freeing stuff\n");
    bvFree(data18);
    bvFree(data20);
    printNodes(head);
    printf("---------------------\n");
    printf("Adding something\n");
    void* data22 = bvMalloc(7, 1);
    printNodes(head);
    printf("---------------------\n");
    printf("Freeing everything left\n"); 
    bvFree(data22);
    bvFree(data17);
    bvFree(data19);
    bvFree(data21);
    printNodes(head);
    printf("---------------------\n");
    
    printf("Testing adding two 500s and a 1\n");
    void* data23 = bvMalloc(500, 1);
    void* data24 = bvMalloc(500, 1);
    void* data25 = bvMalloc(1, 1);
    printNodes(head);
    printf("%p\n", data25);
    printf("---------------------\n");

    bvFree(data23);
    bvFree(data24);


    return 0;
}
