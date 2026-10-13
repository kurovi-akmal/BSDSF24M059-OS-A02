# Operating Systems Programming Assignment 02
## Implementation of the Linux `ls` Utility

## Feature 2: Complete Long Listing Format (-l)

### Question 1: What is the crucial difference between `stat()` and `lstat()`? When is `lstat()` more appropriate?

The `stat()` and `lstat()` system calls are used to retrieve file metadata, such as permissions, file size, owner, and modification time. The main difference is that `stat()` follows a symbolic link and returns information about its target, whereas `lstat()` returns information about the symbolic link itself. In an `ls` implementation, `lstat()` is more appropriate when we need to identify symbolic links and display information about the link rather than its target.

### Question 2: How can bitwise operators and predefined macros extract information from `st_mode`?

The `st_mode` field in the `struct stat` structure contains information about a file's type and permissions. Predefined macros such as `S_ISDIR()` and `S_ISREG()` identify file types. Permission macros such as `S_IRUSR`, `S_IWUSR`, and `S_IXUSR` represent read, write, and execute permissions for the owner. Bitwise AND (`&`) can test whether a permission bit is set. For example, `st.st_mode & S_IRUSR` checks whether the owner has read permission. Similarly, `S_IFDIR` represents the directory file-type bits.

## Feature 3: Column Display — Down Then Across

### Question 1: Explain the general logic for printing items in a down-then-across format. Why is a simple single loop insufficient?

In down-then-across formatting, filenames are displayed in multiple columns, filling each column from top to bottom before moving to the next column. First, all filenames are stored in an array, and the number of rows and columns is calculated. The program then uses the calculated row count to select the correct filename for each position. A simple loop that prints filenames sequentially would place them from left to right, which is horizontal formatting rather than down-then-across formatting.

### Question 2: What is the purpose of `ioctl()`? What are the limitations of a fixed width of 80 columns?

The `ioctl()` system call can retrieve terminal information. Using the `TIOCGWINSZ` request, the program can obtain the terminal's current width and height. This information helps calculate how many columns can fit on the screen. If the program always assumes a width of 80 characters, its output may be poorly aligned or wrap incorrectly on wider or narrower terminals. Detecting the actual terminal width makes the output more adaptable.

## Feature 4: Horizontal Column Display (-x)

### Question 1: Compare down-then-across and horizontal printing. Which requires more pre-calculation and why?

Down-then-across printing is more complex because the program must calculate the number of rows and columns and determine which filenames belong in each position. Horizontal printing is simpler because the program processes filenames sequentially from left to right, adds spacing, and moves to the next line when the terminal width is reached. Therefore, down-then-across printing requires more pre-calculation.

### Question 2: How are the different display modes (-l, -x, and default) managed?

The program uses command-line argument parsing with `getopt()` to identify the requested options. A flag records the selected display mode. When `-l` is provided, the program calls the long-listing function. When `-x` is provided, it calls the horizontal display function. If neither option is selected, it uses the default down-then-across display function.

## Feature 5: Alphabetical Sorting

### Question 1: Why must all directory entries be stored in memory before sorting? What are the drawbacks for directories containing millions of files?

Sorting requires access to the collection of filenames so that their order can be rearranged. The program therefore reads directory entries into a dynamically allocated array before calling `qsort()`. This allows the filenames to be sorted alphabetically before display. However, storing millions of filenames can consume a large amount of memory and increase processing time. Memory allocation may also fail if insufficient memory is available.

### Question 2: What is the purpose and signature of the comparison function used by `qsort()`? Why does it use `const void *` arguments?

The `qsort()` function sorts an array using a comparison function supplied by the programmer. Its comparison function has the general signature `int compare(const void *a, const void *b)`. The function compares two elements and returns a negative value if the first comes before the second, zero if they are equal, or a positive value if the first comes after the second. The `const void *` arguments allow `qsort()` to work with arrays of different data types while preventing the comparison function from modifying the elements through those pointers. For an array of filename pointers, the function converts the arguments to the appropriate pointer type and compares the filenames using a function such as `strcmp()`.

## Feature 6: Colorized Output

### Question 1: How do ANSI escape codes produce terminal colors? Show the sequence for green text.

ANSI escape codes are special character sequences that terminals interpret as formatting instructions. They can change text color, background color, and text style. The sequence `\033[32m` selects green foreground text, while `\033[0m` resets the formatting to its default state. For example:

```c
printf("\033[32mThis text is green\033[0m\n");
```

The first escape sequence enables green text, and the second resets the terminal formatting.

### Question 2: Which permission bits determine whether a file is executable?

The `st_mode` field contains execute-permission bits for the owner, group, and others. These are represented by `S_IXUSR`, `S_IXGRP`, and `S_IXOTH`, respectively. The program can use bitwise AND to check whether any of these bits are set. If at least one execute bit is set, the file has an execute permission for the corresponding category. The file should also be checked with `S_ISREG()` if the intention is to color executable regular files specifically.

## Feature 7: Recursive Listing (-R)

### Question 1: What is a base case in recursion? What stops recursive `ls` from continuing forever?

A base case is a condition that stops a recursive function from making further recursive calls. In recursive directory listing, the program must skip the special entries `.` and `..`, which refer to the current directory and its parent directory. The program recursively processes only valid subdirectories and stops descending when a directory has no further subdirectories to process. This prevents endless recursion through the current and parent directory entries.

### Question 2: Why must the program construct a full path before making a recursive call?

Constructing a full path is necessary so that the program can identify a subdirectory relative to the directory currently being processed. For example, if the current directory is `parent_dir` and a subdirectory is named `subdir`, the correct path is `parent_dir/subdir`. Passing only `subdir` could cause the program to search relative to the process's current working directory instead of the directory being listed. This may result in a failed lookup or the wrong directory being processed. The full path allows `lstat()` to obtain the correct metadata and the recursive function to visit the intended directory.

## Conclusion

In this assignment, the basic Linux `ls` utility was extended with long listing, column formatting, horizontal display, alphabetical sorting, colorized output, and recursive listing. These features demonstrate the use of C programming, system calls, file metadata, dynamic memory, sorting algorithms, command-line argument parsing, and recursion. Git branches, commits, tags, and releases provide a structured way to manage and deliver the project's development.