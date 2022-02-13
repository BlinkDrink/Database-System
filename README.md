
# Database-System
This repository is meant to show an example of working **SQL-like database management system** console application that I created. 
This repository is divided into 4 main segments:
- Database
- Tables
- B+ Tree
- Query processor
## Database
Database class represents the main structure holding all of the tables related to that Database.
This representaton is done by hash-table where each key is a name of a table and the value for that key is the table itself. 
## Table
Here a table class represents an entity used to manage records and pages of records.
- A **Record** is a class representing a single row of a table(*i.e. Name:"John", Surname:"Doe",Salary:1500*). Record has arbitrary data type for each of its columns depending of the table's scheme that it is part of.
- A **Page** is a class that represents a small segment of a table, containing some of the records of that table. Page has maximum number of records that it can hold. So a table with **8** pages, each with maximum number of records **128**, has 1024 records in total. Page is essentially a collection of records, each record having its own unique ID in that page.
## B+ Tree
A **B+ tree** is an [m-ary tree](https://en.wikipedia.org/wiki/M-ary_tree "M-ary tree") with a variable but often large number of children per node. A B+ tree consists of a root, internal nodes and leaves. The root may be either a leaf or a node with two or more children.
`  
| Node Type | Children Type | Min Number of Children | Max Number Of Children | Example b = 7 |
| ----------- | ----------- | ----------- | ----------- | ----------- |  
| Root Node (when its the only node int he tree) | Records | 0 | b - 1 | 0 - 6 
| Root Node | Internal Nodes or Leaf Nodes | 2 | b | 2 - 7 
| Internal Node | Internal Nodes or Leaf Nodes |![\lceil b/2\rceil ](https://wikimedia.org/api/rest_v1/media/math/render/svg/cb0f3f27bd45fd9443cafc552a6c36e7080109bf) | b | 4 - 7 
| Leaf Node | Records | ![\lceil b/2\rceil ](https://wikimedia.org/api/rest_v1/media/math/render/svg/cb0f3f27bd45fd9443cafc552a6c36e7080109bf) | b | 4 - 7 
### Search
```
function search(k) is
    return tree_search(k, root)
```
```
function: tree_search(k, node) is
    if node is a leaf then
        return node
    switch k do
    case k ≤ p[0]
        return tree_search(k, p[0])
    case k[i] < k ≤ k[i+1]
        return tree_search(k, p[i+1])
    case k[d] < k
        return tree_search(k, p[d])

Where p is the array of keys of the current node and d is the order of the tree
```
### Insertion
-   Perform a search to determine what bucket the new record should go into.
-   If the bucket is not full (at most **b-1**  entries after the insertion), add the record.
-   Otherwise,  _before_  inserting the new record
    -   split the bucket.
        -   original node has ⎡(L+1)/2⎤ items
        -   new node has ⎣(L+1)/2⎦ items
    -   Move ⎡(L+1)/2⎤-th key to the parent, and insert the new node to the parent.
    -   Repeat until a parent is found that need not split.
-   If the root splits, treat it as if it has an empty parent and split as outline above.

B-trees grow at the root and not at the leaves.
### How I make use of it in the DBMS application
B+ Trees are great way of implementing a table that has Primary Key assigned to one of its columns because we know the keys should be unique thus each node of the tree can contain only unique keys inside it, giving us an ellegant way to insert, delete, find nodes in **O(log~m~n)** time complexity where **m** is the degree of the tree. Every table that has primary key we will call Indexed table where the index is placed on one of the columns. In short, I am using the B+ Tree only for the tables that are **Indexed**, this way accessing the records at a specified Key becomes very optimal.
## Query Processor
This class represents an entity used for processing queries in the form of a string input (**Mainly WHERE clauses**).
In short, a query object will be initialized with a string, after which the string will be converted to a form that is easier to use in order to compare different WHERE clauses.


