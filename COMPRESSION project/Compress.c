#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct Node
{
    int freq;
    char data;
    struct Node *left, *right, *next;
} node;

typedef struct hashTable
{
    char key;
    char code[130];
    int freq;
} Table;

Table *hash;
int tableSize = 0;
int tableIndex = 0;

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

node *insert(node *root, char data)
{
    tableSize++;
    if (root == NULL)
    {
        root = createNode(data, 1);
    }
    else
    {
        node *newNode = createNode(data, 1);
        newNode->next = root;
        root = newNode;
    }
    return root;
}

int search(node *head, char key)
{
    while (head != NULL)
    {
        if (head->data == key)
            return 1;
        head = head->next;
    }
    return 0;
}

node *incrementFrequency(node *head, char data)
{
    node *temp = head;
    while (temp != NULL)
    {
        if (temp->data == data)
            (temp->freq)++;
        temp = temp->next;
    }
    return head;
}

node *sort(node *head)
{
    node *ptr, *cpt;
    int temp;
    char data;
    ptr = head;
    while (ptr->next != NULL)
    {
        cpt = ptr->next;
        while (cpt != NULL)
        {
            if (ptr->freq > cpt->freq)
            {
                temp = ptr->freq;
                ptr->freq = cpt->freq;
                cpt->freq = temp;
                data = ptr->data;
                ptr->data = cpt->data;
                cpt->data = data;
            }
            cpt = cpt->next;
        }
        ptr = ptr->next;
    }
    return head;
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

void setCode(char data, char code[], int f)
{
    hash[tableIndex].freq = f;
    hash[tableIndex].key = data;
    strcpy(hash[tableIndex].code, code);
    tableIndex++;
}

void appendCharToString(char str[], char ch)
{
    int len = strlen(str);
    str[len] = ch;
    str[len + 1] = '\0';
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
    for (int i = 0; i < tableIndex; i++)
    {
        printf("%c :  %s\n", hash[i].key, hash[i].code);
    }
}

char *getEncodedString(FILE *file)
{
    char ch;
    int encodedSize = 512; // initial buffer size
    char *encodedString = malloc(encodedSize * sizeof(char));
    encodedString[0] = '\0';

    while ((ch = fgetc(file)) != EOF)
    {
        for (int i = 0; i < tableIndex; i++)
        {
            if (hash[i].key == ch)
            {
                int codeLen = strlen(hash[i].code);
                int curLen = strlen(encodedString);
                if (curLen + codeLen >= encodedSize)
                {
                    encodedSize *= 2;
                    encodedString = realloc(encodedString, encodedSize);
                }
                strcat(encodedString, hash[i].code);
                break;
            }
        }
    }
    return encodedString;
}

void writeEncodedToFile(char *encodedString, FILE *file)
{
    unsigned char byte = 0;
    int bitCount = 0;

    for (int i = 0; i < strlen(encodedString); i++)
    {
        byte <<= 1;
        if (encodedString[i] == '1')
            byte |= 1;
        bitCount++;
        if (bitCount == 8)
        {
            fwrite(&byte, sizeof(byte), 1, file);
            byte = 0;
            bitCount = 0;
        }
    }
    if (bitCount > 0)
    {
        byte <<= (8 - bitCount);
        fwrite(&byte, sizeof(byte), 1, file);
    }
    fclose(file);
}
char *readEncodedFromFile(FILE *file)
{ // Reconstruct the bit string from the binary data
    if (!file)
    {
        perror("Failed to open file");
        return NULL;
    }

    // Move the file pointer to the end to determine file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    // Allocate enough memory for bits (8 bits per byte + 1 for null-terminator)
    char *bitString = (char *)malloc(fileSize * 8 + 1);
    if (!bitString)
    {
        perror("Memory allocation failed");
        fclose(file);
        return NULL;
    }

    // Read each byte and convert to bits
    unsigned char byte;
    int bitIndex = 0;
    for (long i = 0; i < fileSize; i++)
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
char *removePadding(char *string, int len)
{
    if (len % 8 != 0)
    {
        int rem = len % 8;
        int paddedLenght = ((len / 8) * 8) + 8;
        int padding = 8 - rem;
        string[paddedLenght - padding] = '\0';
        return string;
    }
    else
    {
        return string;
    }
}
void displayHead(node*head){
    printf("\n");
    while(head!=NULL){
        printf("%c:%d ",head->data,head->freq);
        head=head->next;
    }
    printf("\n");
}

int main()
{
    char filename[25];
    printf("Enter the File Address:");
    scanf("%s",filename);
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Could not open file.\n");
        return 1;
    }

    node *head = NULL, *treeRoot = NULL;
    char ch;
    /////////////////////////////////// Read Input File /////////////////////////////////////////
    while ((ch = fgetc(file)) != EOF)
    {
        if (search(head, ch)) ////////// Increment frequency of previously found character//////////////////
            head = incrementFrequency(head, ch);
        else //////////////// Insert new character found in the file //////////////////
            head = insert(head, ch);
    }
    fclose(file);
    /////////////////////////////////////////////////////////////////////////////////////////////

    head = sort(head);
    char compFileName[25]="";
    int index;
    for (int i = 0; filename[i] != '\0'; i++) {
        if (filename[i] == '\\') {
            index = i; // Update the last found index of the backslash
        }
    }
    filename[index+1]='\0';
    strcat(compFileName,filename);
    strcat(compFileName,"Compressed.bin");
    FILE *compFile = fopen(compFileName, "wb");
    node *temp1 = head;
    int count = 0;
    while (temp1 != NULL)
    {
        count++;
        char str[10];
        sprintf(str, "%d", temp1->freq);
        count += strlen((str));
        count++;

        temp1 = temp1->next;
    }
    fprintf(compFile, "%d\n", count);
    node *temp = head;
    while (temp != NULL)
    {
        fprintf(compFile, "%c%d ", temp->data, temp->freq);
        temp = temp->next;
    }

    treeRoot = createTree(head);
    hash = malloc(tableSize * sizeof(Table));
    char code[130] = "";
    retriveCodeFromTree(code, treeRoot);

    file = fopen("text.txt", "r");
    char *encodedString = getEncodedString(file);
    fclose(file);
    int len = strlen(encodedString);
     fprintf(compFile, "%d\n", len);
    writeEncodedToFile(encodedString, compFile);
    free(encodedString);
    free(hash);
    ///////////////////////////////////////
    system("cls");
    printf("Compressing...");
    sleep(3);
    system("cls");
    printf("\n\n\n\n\n\t\t\t");
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    printf("File Compressed Successfully to -> 'Compressed.bin' check your Directory !!");
    sleep(3);
    return 0;
}
