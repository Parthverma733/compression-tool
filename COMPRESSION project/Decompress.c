#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// Structure for Linked list and huffman tree///
typedef struct Node
{
    int freq;
    char data;
    struct Node *left, *right, *next;
} node;
// Structure of table for storing frequency and huffman codes for each character//
typedef struct hash
{
    char data;
    int freq;
    char code[130];
} hash;

hash *table;
int j = 0;                                   // index to iterate through the table//
void appendCharToString(char str[], char ch) // Function to append character to a string//
{
    int len = strlen(str); // Find the length of the current string
    str[len] = ch;         // Append the character at the end
    str[len + 1] = '\0';   // Add the null terminator
}
void setFreq(char string[]) // Function to set character with there frequency on table structure//
{
    char data = string[0];
    char freq[10] = "";
    for (int i = 1; i < strlen(string); ++i)
    {
        appendCharToString(freq, string[i]);
    }
    table[j].data = data;
    table[j].freq = atoi(freq);
    j++;
}
void splitDataAndFreq(char string[], int len) // Function to seperate data and frequency from metadata//
{
    char str[10] = "";
    for (int i = 0; i < len; ++i)
    {
        if (string[i] == ' ' && string[i - 1] == ' ')
            appendCharToString(str, string[i]);
        else if (string[i] == ' ' && string[i - 1] != ' ')
        {
            setFreq(str);
            str[0] = '\0';
        }

        else
            appendCharToString(str, string[i]);
    }
}
void display()
{
    for (int i = 0; i < j; ++i)
    {
        printf("%c : %d\n ", table[i].data, table[i].freq);
    }
    return;
}
char *readEncodedFromFile(FILE *file, int size) // Function
{
    if (!file)
    {
        perror("failed to open file");
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);
    fseek(file, size, SEEK_SET);
    // Allocate enough memory for bits (8 bits per byte + 1 for null-terminator)
    char *bitString = (char *)malloc((fileSize - size) * 8 + 1);
    if (!bitString)
    {
        perror("Memory allocation failed");
        fclose(file);
        return NULL;
    }

    // Read each byte and convert to bits
    unsigned char byte;
    int bitIndex = 0;
    for (long i = 0; i < fileSize - size; i++)
    {
        fread(&byte, sizeof(byte), 1, file);

        // Convert each byte to bits
        for (int j = 7; j >= 0; j--)
        {
            bitString[bitIndex++] = (byte & (1 << j)) ? '1' : '0';
        }
    }
    bitString[bitIndex] = '\0'; // Null-terminate the string

    fclose(file);
    return bitString;
}
node *createNode(char data, int freq)
{
    node *temp = (node *)malloc(sizeof(node));
    temp->data = data;
    temp->freq = freq;
    temp->left = NULL;
    temp->right = NULL;
    temp->next = NULL;
    return temp;
}
node *insert(node *root, char data, int freq)
{

    if (root == NULL)
    {
        root = createNode(data, freq);
    }
    else
    {
        node *newNode = createNode(data, freq);
        node *temp = root;
        while (temp->next != NULL)
        {
            temp = temp->next;
        }
        temp->next = newNode;
    }
    return root;
}
void displayHead(node *head)
{
    printf("\n");
    while (head != NULL)
    {
        printf("%c:%d ", head->data, head->freq);
        head = head->next;
    }
    printf("\n");
}

node *enqueue(node *Queue, node *newNode)
{
    if (Queue == NULL)
        return newNode;
    else if (Queue->freq >= newNode->freq)
    {
        newNode->next = Queue;
        return newNode;
    }
    else
    {
        node *temp = Queue;
        while (temp->next != NULL && newNode->freq > temp->next->freq)
            temp = temp->next;
        newNode->next = temp->next;
        temp->next = newNode;
    }
    return Queue;
}

node *dequeue(node **Queue)
{
    if (*Queue == NULL)
        return NULL;
    node *temp = *Queue;
    *Queue = (*Queue)->next;
    temp->next = NULL;
    return temp;
}

node *createTree(node *Queue)
{
    node *Node1 = NULL, *Node2 = NULL, *newNode = NULL;
    while (Queue->next != NULL)
    {
        Node1 = dequeue(&Queue);
        Node2 = dequeue(&Queue);
        int Freq = Node1->freq + Node2->freq;
        newNode = createNode('\0', Freq);
        newNode->left = Node2;
        newNode->right = Node1;
        Queue = enqueue(Queue, newNode);
    }
    return Queue;
}
void dis(node *root)
{
    printf("\n\n\n");
    printf(" (%c|%d)\n", root->data, root->freq);
    printf("/    \\\n");
    printf("(%c|%d)  (%c|%d)", root->left->data, root->left->freq, root->right->data, root->right->freq);
    return;
}
void r(node *root)
{
    if (root->left != NULL && root->right != NULL)
    {
        dis(root);
        r(root->left);
        r(root->right);
        return;
    }
    else
    {
        return;
    }
}

void setCode(char data, char code[], int f)
{
    table[j].freq = f;
    table[j].data = data;
    strcpy(table[j].code, code);
    j++;
}

void retriveCodeFromTree(char code[], node *root)
{
    if (root == NULL)
        return;
    if (root->left == NULL && root->right == NULL)
    {
        setCode(root->data, code, root->freq);
        return;
    }
    int len = strlen(code);

    appendCharToString(code, '0');
    retriveCodeFromTree(code, root->left);
    code[len] = '\0';

    appendCharToString(code, '1');
    retriveCodeFromTree(code, root->right);
    code[len] = '\0';
}

void printHashTable()
{
    for (int i = 0; i < j; i++)
    {
        printf("%c :  %s\n", table[i].data, table[i].code);
    }
}
int main()
{
    char filename[25];
    printf("Enter the File Address:");
    scanf("%s", filename);
    table = (hash *)malloc(256 * sizeof(hash));
    FILE *file = fopen(filename, "rb");
    if (file == NULL)
    {
        printf("error opening file!!");
        return 1;
    }
    char line[256];
    fgets(line, sizeof(line), file);
    int n = atoi(line);
    char *buffer = malloc(n + 1); // Allocate memory for `n` characters + null terminator
    size_t bytesRead;

    if (file == NULL)
    {
        printf("Could not open file.\n");
        free(buffer); // Free allocated memory if file opening fails
        return 1;
    }

    // Read `n` characters from the file into the buffer
    bytesRead = fread(buffer, sizeof(char), n, file);
    char line1[256];
    fgets(line1, sizeof(line1), file);
    int len = atoi(line1);
    // Null-terminate the string
    buffer[bytesRead] = '\0';
    splitDataAndFreq(buffer, n + 1);
    display();
    printf("\n");
    ///////// Read encoded string from compresseed file /////////////////
    char *EncodedString = readEncodedFromFile(file, n + strlen(line) + strlen(line1));

    printf("\n%s", EncodedString);
    /// Remove extra padding////////
    if (len % 8 != 0)
    {
        int rem = len % 8;
        int paddedLenght = strlen(EncodedString);
        int padding = 8 - rem;
        EncodedString[paddedLenght - padding] = '\0';
    }
    printf("\n%s", EncodedString);
    /////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    // Clean up

    fclose(file);
    free(buffer);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////// Create huffman tree to retrive code/////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Create Priority Queue using LinkedList//
    node *head = NULL;
    for (int i = 0; i < j; ++i)
    {
        head = insert(head, table[i].data, table[i].freq);
    }
    ///////////////////////////////////////////
    ////Create tree using Priority Queue(Linked List)///
    node *treeRoot = createTree(head);
    // r(treeRoot);
    /////////////////////retrive Code From Huffman Tree////////////////////////////////
    char code[130] = "";
    j = 0;
    retriveCodeFromTree(code, treeRoot);
    // printHashTable();
    ////////////////////////////////////////////////////////////////////////////////////////////
    char orgFileName[25] = "";
    int index;
    for (int i = 0; filename[i] != '\0'; i++)
    {
        if (filename[i] == '\\')
        {
            index = i; // Update the last found index of the backslash
        }
    }
    filename[index + 1] = '\0';
    strcat(orgFileName, filename);
    strcat(orgFileName, "Original.txt");

    FILE *Ofile = fopen(orgFileName, "w");
    if (Ofile == NULL)
    {
        perror("Failed to open compressed file");
        return 1;
    }

    int tableIndex = j;
    char string[130] = "";
    for (int i = 0; i < strlen(EncodedString); ++i)
    {

        appendCharToString(string, EncodedString[i]);
        for (int j = 0; j < tableIndex; ++j)
        {
            if (strcmp(table[j].code, string) == 0)
            {
                fputc(table[j].data, Ofile);
                string[0] = '\0';
                break;
            }
        }
    }
    fclose(Ofile);


    system("cls");
    printf("Decompressing...");
    sleep(3);
    system("cls");
    printf("\n\n\n\n\n\t\t\t");
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    printf("File Decompressed Successfully to -> 'Original.txt' check your Directory !!");
    sleep(3);
    return 0;
}