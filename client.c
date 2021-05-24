#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

void getUserInput();
void proceedUserInput(int);
void checkoutReservation();
void getDailyCheckout();
void getAvailableTables();


int main(void)
{
    getUserInput();
    return 0;
}

void getUserInput(){

int userInput;

do{
    printf("\n\nMenu:");
    printf("\n1. Show available tables");
    printf("\n2. Checkout reservation");
    printf("\n3. Show daily total income");
    printf("\n4. Exit");
    printf("\nEnter your choise: ");
    scanf("\%d", &userInput);
    while (getchar() != '\n');
    fflush(stdin);

    proceedUserInput(userInput);
    }while(userInput != 0);
}

void proceedUserInput(int userInput){
    switch(userInput){
        case 1 :
        getAvailableTables();
        break;
        case 2:
        checkoutReservation();
        break;
        case 3:
        getDailyCheckout();
        default:
        return;
    }

}

void getAvailableTables(){
   int socket_desc;
    struct sockaddr_in server_addr;
    char server_message[2000], client_message[2000];

    // Clean buffers:
    memset(server_message,'\0',sizeof(server_message));
    memset(client_message,'\0',sizeof(client_message));

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if(socket_desc < 0){
        printf("Unable to create socket\n");
        return;
    }

    printf("Socket created successfully\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2001);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Send connection request to server:
    if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Unable to connect\n");
        return;
    }
    printf("Connected with server successfully!!!\n");



    // Send the message to server:
    strcpy(client_message, "GET_TABLES");
    if(send(socket_desc, client_message, strlen(client_message), 0) < 0){
        printf("Unable to send message\n");
        return;
    }

    if(recv(socket_desc, server_message, sizeof(server_message), 0) < 0){
        printf("Error while receiving server's msg\n");
        return;
    }
    printf("Server's response: %s\n",server_message);
     memset(server_message,'\0',sizeof(server_message));
    memset(client_message,'\0',sizeof(client_message));

    // Get input from the user:
    printf("Enter message: ");
    fflush(stdin);
    fgets(client_message, sizeof(client_message), stdin);
    printf("\n%s", client_message);

    // Send the message to server:
    if(send(socket_desc, client_message, strlen(client_message), 0) < 0){
        printf("Unable to send message\n");
        return -1;
    }

    int flag=0;

    while(1){

      if(recv(socket_desc, server_message, sizeof(server_message), 0) < 0){
        printf("Error while receiving server's msg\n");
        return -1;
    }

    flag++;
        if(strcmp(server_message, "SUCCESS")==0){
            printf("Request completed successfully");
            memset(server_message, '\0', sizeof(server_message));
            if(flag<2){
                printf("\nNo free places for this date!!!");
            }
            flag = 0;
            break;
        }

        // Receive the server's response:
        server_message[strlen(server_message)-1] ='\0';
        printf("%s\n",server_message);



        memset(server_message, '\0', sizeof(server_message));
        usleep(1000);

    }


    // Close the socket:
    close(socket_desc);
}

void checkoutReservation(){
int socket_desc;
    struct sockaddr_in server_addr;
    char server_message[50], client_message[50];

    // Clean buffers:
    memset(server_message,'\0',sizeof(server_message));
    memset(client_message,'\0',sizeof(client_message));

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if(socket_desc < 0){
        printf("Unable to create socket\n");
        return;
    }

    printf("Socket created successfully\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2001);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Send connection request to server:
    if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Unable to connect\n");
        return;
    }
    printf("Connected with server successfully\n");

    strcpy(client_message, "CHECKOUT_RES");

    // Send request to server:
    if(send(socket_desc, client_message, strlen(client_message), 0) < 0){
        printf("Unable to send message\n");
        close(socket_desc);
        return;
    }

    // Get input from the user
    memset(client_message, '\0', sizeof(client_message));
    printf("Enter reservation_id to checkout: ");
    gets(client_message);

    if(send(socket_desc, client_message, strlen(client_message), 0) < 0){
        printf("Unable to send message\n");
        close(socket_desc);
        return;
    }

    // Get input from the user
    printf("Enter total to checkout: ");
    gets(client_message);

    if(send(socket_desc, client_message, strlen(client_message), 0) < 0){
        printf("Unable to send message\n");
        close(socket_desc);
        return;
    }

    //Receive server response
     memset(server_message, '\0', sizeof(server_message));

     if(recv(socket_desc, server_message, sizeof(server_message), 0) < 0){
        printf("Error while receiving server's response\n");
        close(socket_desc);
        return;
    }

    if(strcmp(server_message, "SUCCESS") == 0){
        printf("\nRequest processed successfully");
        close(socket_desc);
        return;
    }else{
        printf("\nError occured, please try again later");
        close(socket_desc);
        return;
    }

}
void getDailyCheckout(){
    int socket_desc;
    struct sockaddr_in server_addr;
    char server_message[2000], client_message[100];

    // Clean buffers:
    memset(server_message,'\0',sizeof(server_message));
    memset(client_message,'\0',sizeof(client_message));

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if(socket_desc < 0){
        printf("Unable to create socket\n");
        return;
    }

    printf("Socket created successfully\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2001);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Send connection request to server:
    if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Unable to connect\n");
        return;
    }
    printf("Connected with server successfully\n");

    strcpy(client_message, "CHECKOUT_DAILY");

    // Send request to server:
    if(send(socket_desc, client_message, strlen(client_message), 0) < 0){
        printf("Unable to send message\n");
        close(socket_desc);
        return;
    }

    if(recv(socket_desc, server_message, sizeof(server_message), 0) < 0){
        printf("Error while receiving server's resonse\n");
        close(socket_desc);
        return;
    }
    printf("Server's response: %s\n",server_message);
     memset(server_message,'\0',sizeof(server_message));
    memset(client_message,'\0',sizeof(client_message));

    // Get input from the user:
    printf("Enter date to see total (format YYYY-MM-DD): ");
    gets(client_message);

    // Send the message to server:
    if(send(socket_desc, client_message, strlen(client_message), 0) < 0){
        printf("Unable to send message\n");
        close(socket_desc);
        return;
    }

    while(1){

      if(recv(socket_desc, server_message, sizeof(server_message), 0) < 0){
        printf("Error while receiving server's response\n");
        close(socket_desc);
        return;
    }
        if(strcmp(server_message, "FAIL") == 0){
            printf("\nServer error");
            close(socket_desc);
            return;
        }
        if(strcmp(server_message, "SUCCESS")==0){
            printf("Request completed successfully");
            memset(server_message, '\0', sizeof(server_message));
            close(socket_desc);
            break;
        }
        server_message[strlen(server_message)-1] ='\0';
        printf("%s\n",server_message);

        memset(server_message, '\0', sizeof(server_message));
        usleep(1000);
    }
    close(socket_desc);
}
