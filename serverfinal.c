#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <mysql/mysql.h>
#include <arpa/inet.h>
#include <signal.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#define SERVER_SQL "127.0.0.1"
#define USER_SQL "root"
#define PASSWORD_SQL "root"
#define DATABASE_SQL "mess"
#define PORT 8745

void sigchld_handler (int signal)  // handler for the parent process in order to avoid defunct processes
{
    int child_status_code;
    wait(&child_status_code);
    printf("SIGCHLD handler with the exit status : %d \n", child_status_code);
}

void receiveCommand (char * comm, int client, size_t maxBytes)
{
    bzero(comm, maxBytes);
    read(client, comm, maxBytes);
}

int readMessage(char * message, int client, size_t maxBytes)
{
    bzero(message, maxBytes);
    if(read(client, message, maxBytes) == -1)
    {
        return -1;
    }
    else
{
    return 1;
}
}

void sendAnswer( void * answer, int client, size_t length) // sending the answer processed by processCommand back to our client
{  
    write(client, answer, length); 
    close(client);

}

void finish_with_error(MYSQL *con)
{
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  exit(1);
}

void deleteNewline( char * query)
{
    int len = strlen(query);
    if(query[len-1] == '\n')
    {
        query[len-1]=0;
        printf("era enter dupa username \n");

    }
}

int loginUser(char * username, MYSQL * con)
{
    int userExists = 0;
    char query1[255] = "select * from users where username='";
    char * query2;
    query2 = malloc(strlen(query1) + strlen(username) + 2);
    strcpy(query2, query1);
    strcat(query2, username);
    deleteNewline(query2);
    strcat(query2, "';");
    printf("interogarea: %s \n", query2);

    mysql_query(con, query2);
    MYSQL_RES *query_result = mysql_store_result(con);
    printf("nr coloane rezultate: %d \n", mysql_field_count(con));
    MYSQL_ROW row; 
    int num_fields = mysql_num_fields(query_result);

    while ((row = mysql_fetch_row(query_result))) {

        for(int i = 0; i < num_fields; i++)
      {
          printf("%s ", row[i] ? row[i] : "NULL");
      }

      printf("\n");
      printf("inainte de if-ul in care compar row[1] cu username, userExists este %d \n", userExists);
        userExists = 1;
        break;
    }
    mysql_free_result(query_result);
    
return userExists;
}

void modifyStatus(char * username, MYSQL * con )
{
    char end[10]="';";
    char query1[255] = "update users set active=1 where username='";
    char * query2;
    query2 = malloc(strlen(query1) + strlen(username) + 2);
    strcpy(query2, query1);
    strcat(query2, username);
    deleteNewline(query2);
    strcat(query2, end);
    printf("interogarea pt status: %s \n", query2);

    mysql_query(con, query2);
    MYSQL_RES *result = mysql_store_result(con);
    mysql_free_result(result);

}

int returnStatus ( char * username, MYSQL * con)
{
    int status=-1;
    char end[10]="';";
    char query1[255] = "select active from users where username='";
    char * query2;
    query2 = malloc(strlen(query1) + strlen(username) + 2);
    strcpy(query2, query1);
    strcat(query2, username);
    deleteNewline(query2);
    strcat(query2, end);
    printf("interogarea pt returnstatus: %s \n", query2);

    mysql_query(con, query2);
    MYSQL_RES *result = mysql_store_result(con);

    int num_fields = mysql_num_fields(result);

  MYSQL_ROW row;

  while ((row = mysql_fetch_row(result)))
  {
      for(int i = 0; i < num_fields; i++)
      {
          status=atoi(row[i]);
      }

  }
    mysql_free_result(result);
    return status;
}

int signupUser( char * username, MYSQL * con)
{
    int userExists = loginUser (username, con);
    printf("userExists din signup: %d \n", userExists);

    if (userExists == 0)
    {
        char query1[255] = "insert into users (username, active) values ('";
        char end[10] = "', 0);";
        char * query2;
        query2 = malloc(strlen(query1) + strlen(username) + strlen(end));
        strcpy(query2, query1);
        strcat(query2, username);
        deleteNewline(query2);
        strcat(query2, end);

        mysql_query(con, query2);
        
    }
    return userExists;
}

char * getLoggedInUser (char * comm, char * username)
{
    strcpy(username, comm+6);
    return username;
}

int getUserid (char * username, MYSQL * con) // receiving a char with a username, it returns the user's id as an int
{   int id;
    char end[10]="';";
    char query1[255] = "select user_id from users where username='";
    char * query2;
    query2 = malloc(strlen(query1) + strlen(username) + 2);
    strcpy(query2, query1);
    strcat(query2, username);
    deleteNewline(query2);
    strcat(query2, end);
    mysql_query(con, query2);
    MYSQL_RES *result = mysql_store_result(con);

    int num_fields = mysql_num_fields(result);

  MYSQL_ROW row;

  while ((row = mysql_fetch_row(result)))
  {
      for(int i = 0; i < num_fields; i++)
      {
          id=atoi(row[i]);
      }
  }
    mysql_free_result(result);
    return id;

}

int sendMessage(int convo_id, int client, char * sender, char * receiver, MYSQL * con)
{
    char controlMessage[255];
    bzero(controlMessage, sizeof(controlMessage));
    strcat(controlMessage, "message:success");
    int length_answer = strlen(controlMessage);
    controlMessage[length_answer] = '\0';            
    write(client, controlMessage, length_answer ); // trimit raspuns la client


    printf("in sendmessage \n");
    char message[255];
    char convo_ids[5];
    int convo=0;
    int len = read(client, message, 255);
    message[len] = '\0';
    printf("len: %d \n", len);
    deleteNewline(message);
    printf("mesajul de la user: %s \n", message);
    // urmeaza sa introduc mesajul in baza de date daca nu e "stop conversation"
    if(strstr(message, "stop conversation"))
    {
        convo=-1;
        
    }
    else
    {
        char query1_insert[255] = "insert into messages (convo_id, message, sender_id, sent_to_id) values (";
        char comma[2]=",";
        char sep_mess[2]="'";
        char end_insert[10] = ");";
        char * query2_insert;
        query2_insert = malloc(strlen(query1_insert) + strlen(message)  + strlen(end_insert) + 4);
        strcpy(query2_insert, query1_insert);
        sprintf(convo_ids, "%d", convo_id);
        strcat(query2_insert, convo_ids);
        strcat(query2_insert, comma);
        strcat(query2_insert, sep_mess);
        strcat(query2_insert, message);
        strcat(query2_insert, sep_mess);
        strcat(query2_insert, comma);
        strcat(query2_insert, sender);
        strcat(query2_insert, comma);
        strcat(query2_insert, receiver);
        strcat(query2_insert, end_insert);

    
        printf("interogarea pt sendmessage: %s \n", query2_insert);

        if(mysql_query(con, query2_insert) == 0)
        {
            sendMessage(convo_id, client, sender, receiver, con);
        }
        printf("am facut insertul in sendmessage \n");

    }

      
     return convo;
    
}

char * getUsername (char * id, MYSQL * con)
{
    char username[15];
    char end[10]=";";
    char query1[255] = "select username from users where user_id=";
    char * query2;
    query2 = malloc(strlen(query1) + strlen(id) + 2);
    strcpy(query2, query1);
    strcat(query2, id);
    strcat(query2, end);
    printf("interogarea pt getusername: %s \n", query2);

    mysql_query(con, query2);
    MYSQL_RES *result = mysql_store_result(con);

    int num_fields = mysql_num_fields(result);

  MYSQL_ROW row;

  while ((row = mysql_fetch_row(result)))
  {
      strcpy(username, row[0]);
  }
   mysql_free_result(result);
  char * getusername = malloc(strlen(username) + 1);
  strcpy(getusername, username);
  printf("username din getusername: %s \n", getusername);
  return getusername;

}

char * seeHistory(char * loggedUser, char * comm, MYSQL * con)
{

    int id1, id2;
    char friend[15];
    char retMessage2[1000];
    retMessage2[0]='\0';
    strcpy(friend, comm + 17);
    deleteNewline(friend);
    printf("friend din see history este: %s \n", friend);

    id1 = getUserid(loggedUser, con);
    id2 = getUserid(friend, con);
    char query1[255] = "select convo_id from convos where user_id=";
    char query2[255] = " and friend_id=";
    char end[5] = ";";
    int convo_id = -1;
    char * query3;
     query3 = malloc(strlen(query1) + strlen(query2) + 3);
    strcpy(query3, query1);
    char user_id[5], friend_id[5];

    sprintf(user_id, "%d", id1);
    sprintf(friend_id, "%d", id2);
    
    strcat(query3, user_id);
    strcat(query3, query2);
    strcat(query3, friend_id);
    strcat(query3, end);
    printf("interogare 1 din see history: %s \n", query3);
    mysql_query(con, query3);
    MYSQL_RES *result = mysql_store_result(con);

    int num_fields = mysql_num_fields(result);

  MYSQL_ROW row;

  while ((row = mysql_fetch_row(result)))
  {
      for(int i = 0; i < num_fields; i++)
      {
          convo_id=atoi(row[0]);
        
      }

    
  }
  mysql_free_result(result);
  if (convo_id == -1)
  {
      printf("nu am gasit rezultat  \n");
      char query1_r[255] = "select convo_id from convos where user_id=";
      char query2_r[255] = " and friend_id=";
      char * query3_r;
      query3_r = malloc(strlen(query1_r) + strlen(query2_r) + 3);
      strcpy(query3_r, query1_r);
      strcat(query3_r, friend_id);
      strcat(query3_r, query2_r);
      strcat(query3_r, user_id);
      strcat(query3_r, end);
      printf("sa vedem inversul din see history: %s \n", query3_r);
      mysql_query(con, query3_r);
      MYSQL_RES *result = mysql_store_result(con);

      int num_fields = mysql_num_fields(result);

  MYSQL_ROW row;

  while ((row = mysql_fetch_row(result)))
  {
      printf("intru in a doua interogare \n");
      for(int i = 0; i < num_fields; i++)
      {
          convo_id=atoi(row[0]);
          printf("%s ", row[i] ? row[i] : "NULL");
      }

  }
  mysql_free_result(result);
  }

  if(convo_id == -1)
  {
    printf("nu am istorie  \n");

  }
  else
  {
    char querym[255] = "select message, sender_id from messages where convo_id=";
    char endm[10]=";";
    char * query2m;
     query2m = malloc(strlen(querym) + 3);
    strcpy(query2m, querym);
    char convo_idm[5];
    char selMessage[255];

    sprintf(convo_idm, "%d", convo_id);
    
    strcat(query2m, convo_idm);
    strcat(query2m, end);
    printf("interogare select mess din see history: %s \n", query2m);
    mysql_query(con, query2m);
    printf("checkpoint \n");
    MYSQL_RES *result = mysql_store_result(con);

    int num_fields = mysql_num_fields(result);

  MYSQL_ROW row;
  char id_sender[15];
  char istoric[15]="\n HISTORY: \n ";


  while ((row = mysql_fetch_row(result)))
  {
         strcpy(selMessage, row[0]);
         printf("mesajul selectat este: %s \n", selMessage);
         char * user_sender = getUsername(row[1], con);
         strcpy(id_sender, user_sender);
         printf("id_sender selectat este: %s \n", id_sender);
         strcat(retMessage2, selMessage);
         strcat(retMessage2, " -> sent by ");
         strcat(retMessage2, id_sender);
         strcat(retMessage2, "\n");
         strcat(retMessage2, "\n");

  }
  char * finalHistory = malloc(strlen(retMessage2) + strlen(istoric));
  printf("arata mi retMessage nou: %s \n", retMessage2);
  printf("checkpoint \n ");
  strcpy(finalHistory, istoric);
  strcat(finalHistory, retMessage2);
  mysql_free_result(result);
return finalHistory;
  }

}



int startConversation( char * loggedUser, char * comm, MYSQL * con, int client)
{  
    char message[255];
    int id1, id2;
    int convo_id = -1;
    char friend[15];
    char end[10]=";";
    char query1[255] = "select convo_id from convos where user_id=";
    char query2[255] = " and friend_id=";
    char * query3;
    id1 = getUserid(loggedUser, con);
    printf("id eu: %d \n", id1);
     printf("comm este: %s \n", comm);
    strcpy(friend, comm + 24);
    deleteNewline(friend);
    printf("friend este: %s \n", friend);
    id2 = getUserid(friend, con);
    printf("id friend: %d \n", id2);
    // at this point we memorised our id and our friend's id

    query3 = malloc(strlen(query1) + strlen(query2) + 3);
    strcpy(query3, query1);
    char user_id[5], friend_id[5];

    sprintf(user_id, "%d", id1);
    sprintf(friend_id, "%d", id2);
    
    strcat(query3, user_id);
    strcat(query3, query2);
    strcat(query3, friend_id);
    strcat(query3, end);
    printf("sa vedem: %s \n", query3);
    mysql_query(con, query3);
    MYSQL_RES *result = mysql_store_result(con);

    int num_fields = mysql_num_fields(result);

  MYSQL_ROW row;

  while ((row = mysql_fetch_row(result)))
  {
      for(int i = 0; i < num_fields; i++)
      {
          convo_id=atoi(row[0]);
      }

  }
  mysql_free_result(result);
  if (convo_id == -1)
  {
      printf("nu am gasit rezultat  \n");
      char query1_r[255] = "select convo_id from convos where user_id=";
      char query2_r[255] = " and friend_id=";
      char * query3_r;
      query3_r = malloc(strlen(query1_r) + strlen(query2_r) + 3);
      strcpy(query3_r, query1_r);
      strcat(query3_r, friend_id);
      strcat(query3_r, query2_r);
      strcat(query3_r, user_id);
      strcat(query3_r, end);
      printf("sa vedem inversul: %s \n", query3_r);
      mysql_query(con, query3_r);
      MYSQL_RES *result = mysql_store_result(con);

      int num_fields = mysql_num_fields(result);

  MYSQL_ROW row;

  while ((row = mysql_fetch_row(result)))
  {
      printf("intru in a doua interogare \n");
      for(int i = 0; i < num_fields; i++)
      {
          convo_id=atoi(row[0]);
          printf("%s ", row[i] ? row[i] : "NULL");
      }

  }
  mysql_free_result(result);

  if(convo_id == -1)
  {
      char query1_insert[255] = "insert into convos (user_id, friend_id) values (";
      char comma[2]=",";
      char end_insert[10] = ");";
      char * query2_insert;
        query2_insert = malloc(strlen(query1_insert) + strlen(user_id) + strlen(friend_id) + strlen(end));
        strcpy(query2_insert, query1_insert);
        strcat(query2_insert, user_id);
        strcat(query2_insert, comma);
        strcat(query2_insert, friend_id);
        strcat(query2_insert, end_insert); 
        printf("interogarea pt insertconvo: %s \n", query2_insert);

        mysql_query(con, query2_insert);
        printf("am facut insertul \n");
        mysql_query(con, query3);
        MYSQL_RES *result = mysql_store_result(con);

    int num_fields = mysql_num_fields(result);

  MYSQL_ROW row;

  while ((row = mysql_fetch_row(result)))
  {
      for(int i = 0; i < num_fields; i++)
      {
          convo_id=atoi(row[0]);
      }
  }
  
  }
    printf("convo id este: %d \n", convo_id);

  }
  
printf("inainte de sendmessage \n");
if(sendMessage(convo_id, client, user_id, friend_id, con) == -1)
        return convo_id;
printf("dupa  sendmessage \n");



}

char * lastActive(char * comm, MYSQL * con )
{
    char username[15];
    char date[100];
    char user_id[5];
    strcpy(username, comm + 12);
    deleteNewline(username);
    int id;
    char query1[255] = "select date_m from messages where sender_id=";
    char query3[255] = " order by id_m desc limit 1;";
    char * query2;
    query2 = malloc(strlen(query1) + strlen(query3) + 2); 
    id = getUserid(username, con);
    sprintf(user_id, "%d", id);
    strcpy(query2, query1);
    strcat(query2, user_id);
    strcat(query2, query3);
    printf("interogarea pt lastActive: %s \n", query2);

    mysql_query(con, query2);
    MYSQL_RES *result = mysql_store_result(con);

    int num_fields = mysql_num_fields(result);

  MYSQL_ROW row;

  while ((row = mysql_fetch_row(result)))
  {
      strcpy(date, row[0]);

    //   printf("\n");
  }
  printf("date este:: %s \n", date);
    char * date_final = malloc(strlen(date) + 1);
    strcpy(date_final, date);
    mysql_free_result(result);
    printf("date_final este: %s \n", date_final);
    return date_final;
}

int logout (char * comm, char * loggedUser, MYSQL * con)
{
    int userLoggedIn = -1;
    char username[15];
    strcpy(username, comm + 7);
    deleteNewline(username);
    if(strcmp(username, loggedUser) == 0)
    {
        userLoggedIn = 1;
    }
    else
    {
        userLoggedIn = 0;
    }
    printf("userLoggedIn in logout: %d \n", userLoggedIn);
    
    if(userLoggedIn == 1)
    {
        char query1[255] = "update users set active=0 where username='";
        char end[10] = "';";
        char * query2;
        query2 = malloc(strlen(query1) + strlen(username) + strlen(end));
        strcpy(query2, query1);
        strcat(query2, username);
        strcat(query2, end);
        printf("interogarea pt logout: %s \n", query2);

        mysql_query(con, query2);
    }
    

    return userLoggedIn; // 1 if the logout command can be done correctly,
                         // 0 if the username in the logout command was wrong,
                         // -1 if you're not even logged in
}

char * checkNewMessages (char * comm, MYSQL * con)
{
    char username[15];
    char date[100];
    char user_id[5];
    strcpy(username, comm + 23);
    deleteNewline(username);
    int id;
    char query1[255] = "select date_m from messages where sender_id=";
    char query3[255] = " order by id_m desc limit 1;";
    char * query2;
    query2 = malloc(strlen(query1) + strlen(query3) + 2); 
    id = getUserid(username, con);
    sprintf(user_id, "%d", id);
    strcpy(query2, query1);
    strcat(query2, user_id);
    strcat(query2, query3);
    printf("interogarea 1 pt checknew: %s \n", query2);

    mysql_query(con, query2);
    MYSQL_RES *result = mysql_store_result(con);

    int num_fields = mysql_num_fields(result);

  MYSQL_ROW row;

  while ((row = mysql_fetch_row(result)))
  {
      strcpy(date, row[0]);

    //   printf("\n");
  }
   mysql_free_result(result);
  char query1_new[255] = "select message, sender_id from  convos join messages on convos.convo_id=messages.convo_id where sent_to_id=";
  char query2_new[255] = " and date_m > '";
  char end_new[5] = "';";
  char * query3_new;
  char selMessage[100];
  char retMessage2[1000];
  query3_new = malloc(strlen(query1_new) + strlen(query2_new) + strlen(end_new) + 1);
  strcpy(query3_new, query1_new);
  strcat(query3_new, user_id);
  strcat(query3_new, query2_new);
  strcat(query3_new, date);
  strcat(query3_new, end_new);
  printf("interogarea 2 pt checknew: %s \n", query3_new);

  mysql_query(con, query3_new);
    MYSQL_RES *result1 = mysql_store_result(con);

    int num_fields1 = mysql_num_fields(result1);

  MYSQL_ROW row1;
  char id_sender[15];
  char first[50] = "\n You have ";
  int noMess = 0;

  while ((row1 = mysql_fetch_row(result1)))
  {
        noMess = noMess + 1;
         strcpy(selMessage, row1[0]);
         printf("mesajul selectat este: %s \n", selMessage);
         strcpy(id_sender, getUsername(row1[1], con));
         printf("id_sender selectat este: %s \n", id_sender);
         strcat(retMessage2, selMessage);
         strcat(retMessage2, " -> sent by ");
         strcat(retMessage2, id_sender);
         strcat(retMessage2, "\n");
         strcat(retMessage2, "\n");

  }
  printf("arata mi newMessages nou: %s \n", retMessage2);
  printf("ajungi pana aici? \n ");
  char noMess1[5];
  sprintf(noMess1, "%d", noMess);
  strcat(first, noMess1);
  strcat(first, " new messages! \n");
  char * newMessages = malloc(strlen(retMessage2) + strlen(first));
  strcpy(newMessages, first);
  strcat(newMessages, retMessage2);
  mysql_free_result(result1);
return newMessages;


}

void processCommand (char * answer, char * comm, int client, MYSQL * con, int fd[2])
{
    int userExists;
    char * loggedUser = NULL;
    char pipetest[100];
    if(strstr(comm, "login"))
                {
                    char username [15];
                    char username2 [15];
                    strcpy(username, comm+6); 
                    deleteNewline(username);
                   
                    
                    userExists = loginUser(username, con);
                    printf("userExists este: %d \n", userExists);
                    if (userExists == 1)
                    {
                        bzero(answer, sizeof(answer));
                        strcat(answer, "login:success");
                        modifyStatus(username, con); 
                        returnStatus(username, con);
                        loggedUser = getLoggedInUser(comm, username2);
                        deleteNewline(loggedUser);
                        printf("loggedUser este: %s \n", loggedUser);
                        write(fd[1], loggedUser, strlen(loggedUser)+1);


                    }
                    else
                    {
                        bzero(answer, sizeof(answer));
                        strcat(answer, "login:failed");
                    }
                
                    int length_answer = strlen(answer);
                    answer[length_answer] = '\0';
                    write(client, answer, length_answer ); // trimit raspuns la client

                }

    else if(strstr(comm, "status"))
                 
                 {
                    char username [15];
                    int status;
                    strcpy(username, comm+7);
                    status = returnStatus(username, con);
                    if (status == 1)
                    {
                        bzero(answer, sizeof(answer));
                        strcat(answer, "status:active");
                    } 
                    else
                    {
                        bzero(answer, sizeof(answer));
                        strcat(answer, "status:offline");
                    }

                    int length_answer = strlen(answer);
                    answer[length_answer] = '\0';
                    write(client, answer, length_answer ); // trimit raspuns la client
                     
                     
                    

                }
    else if (strstr(comm, "sign up")) 
    {
        char username[15];
        strcpy(username, comm+8);
        printf("username la signup este: %s \n", username);
        int user = signupUser(username, con);
        printf("user la signup  este: %d \n", user);
        if(user == 1)
        {
            bzero(answer, sizeof(answer));
            strcat(answer, "signup:failed");

        }
        else
        {
            bzero(answer, sizeof(answer));
            strcat(answer, "signup:success");

        }

        int length_answer = strlen(answer);
        answer[length_answer] = '\0';
        write(client, answer, length_answer ); // trimit raspuns la client
        
    }

    else if (strstr(comm, "start conversation with "))
    {
        
        int end = 0; 
        read(fd[0], pipetest, 100); // reading the username that i'm logged in as in this session from the pipe
        write(fd[1], pipetest, strlen(pipetest)+1);
        end = startConversation(pipetest, comm, con, client);
        if (end != 0)
        {   
            bzero(answer, sizeof(answer));
        strcat(answer, "conversation:success");
        int length_answer = strlen(answer);
        answer[length_answer] = '\0';
        write(client, answer, length_answer ); // trimit raspuns la client
        }

    }
    else if (strstr(comm, "see history with "))
    {
        printf("checkpoint \n ");
        if(userExists != 1)
        {
            printf("checkpoint \n ");
            bzero(answer, sizeof(answer));
        strcat(answer, "history:failed");
        int length_answer = strlen(answer);
        answer[length_answer] = '\0';
        write(client, answer, length_answer ); // trimit raspuns la client
        }
        else
        {
            char history2[1000];
            char pipetest2[100];
        
            bzero(history2, sizeof(history2));
            read(fd[0], pipetest2, 100); // reading the username that i'm logged in as in this session from the pipe
            write(fd[1], pipetest2, strlen(pipetest2)+1);
            char * history = seeHistory(pipetest2, comm, con);
            printf("history e: %s \n ", history);
            bzero(answer, sizeof(answer));
            strcat(answer, history);
            int length_answer = strlen(answer);
            answer[length_answer] = '\0';
            write(client, answer, length_answer ); // trimit raspuns la client
            // free(history);
        }
        

        
    }
    else if(strstr(comm, "last active "))
    {
        char * last_active = lastActive(comm, con);
        bzero(answer, sizeof(answer));
        strcat(answer, last_active);
        int length_answer = strlen(answer);
        answer[length_answer] = '\0';
        write(client, answer, length_answer ); // trimit raspuns la client
    }

    else if(strstr(comm, "logout "))
    {
        if(userExists != 1)
        {
            bzero(answer, sizeof(answer));
            strcat(answer, "logout:failed:login");
            int length_answer = strlen(answer);
            answer[length_answer] = '\0';
            write(client, answer, length_answer ); // trimit raspuns la client
        }
        else
        {
            read(fd[0], pipetest, 100); // reading the username that i'm logged in as in this session from the pipe
            write(fd[1], pipetest, strlen(pipetest)+1);
            printf("loggedUser din logout: %s \n ", pipetest);
            int checkLogout = logout(comm, pipetest, con);
            if(checkLogout == 1)
            {
                bzero(answer, sizeof(answer));
                strcat(answer, "logout:success");
                int length_answer = strlen(answer);
                answer[length_answer] = '\0';
                write(client, answer, length_answer ); // trimit raspuns la client

            }
            else if(checkLogout == 0)
            {
                bzero(answer, sizeof(answer));
                strcat(answer, "logout:failed");
                int length_answer = strlen(answer);
                answer[length_answer] = '\0';
                write(client, answer, length_answer ); // trimit raspuns la client

            }
            else if(checkLogout == -1)
            {
                bzero(answer, sizeof(answer));
                strcat(answer, "logout:nologin");
                int length_answer = strlen(answer);
                answer[length_answer] = '\0';
                write(client, answer, length_answer ); // trimit raspuns la client
            }
            
        }
    }
    else if(strstr(comm, "check new messages for "))
    {
        char * newMessages = checkNewMessages(comm, con);
            printf("mesajele noi: %s \n ", newMessages);
            bzero(answer, sizeof(answer));
            strcat(answer, newMessages);
            int length_answer = strlen(answer);
            answer[length_answer] = '\0';
            write(client, answer, length_answer ); // trimit raspuns la client
            // free(newMessages);
    }
    
}

MYSQL * connectToDatabase()
{
     MYSQL *con = mysql_init(NULL);
     if(mysql_real_connect(con, SERVER_SQL, USER_SQL, PASSWORD_SQL, DATABASE_SQL, 0, NULL, 0) == NULL) 
     {
        perror("Couldn't connect to the database. \n");
        mysql_close(con);
        exit(1);
     }

     else 
     {
          printf("Successfully connected to the database. \n ");
          return con;
         
     }
}



int main () 
{
    

    signal(SIGCHLD,sigchld_handler); // explaining how to handle SIGCHLD when needed

    int check = -1;
    struct sockaddr_in server_address, client_address;

    // Creating the socket 

   int  sock_server = socket(AF_INET, SOCK_STREAM, 0); // file descriptor of the main socket 

    // Error handling for socket 

    if(sock_server < 0 ) 
    {
        perror("Failed at socket()");
        return -1;
    }

    // Populating the server_address struct 

    server_address.sin_family= AF_INET;
    server_address.sin_port=htons(PORT); 
    server_address.sin_addr.s_addr=htonl(INADDR_ANY); //  converts the string into the standard ipv4 decimal notation 

    // Bind  

   check = bind(sock_server, (struct sockaddr *)&server_address, sizeof(server_address));

    // Error handling for bind
    if (check < 0)
    {
        perror("Failed at bind()");
        close(sock_server);
        return -1;
    }

    // The server's socket starts listening for new client requests for connections

    check = listen(sock_server, 8);

    // Error handling for listen 

    if (check < 0)
    { 
        perror("Failed at listen()");
        close(sock_server);
        return -1;
    }

    printf("Parent process id is %d \n", getpid());

    // Accepting new client requests
    
    while(1) {
        socklen_t client_length= sizeof(client_address);
        int sock_client = accept(sock_server, (struct sockaddr *)&client_address, &client_length);
         // file descriptor of the socket generated when a client request for connection is accepted

        // Error handling for accept()

        if(sock_client < 0) 
        {
            perror("Failed at accept()");
            close(sock_server);
            return -1;
        }

        // Creating a child process for each client that we just accepted

        pid_t pid;
        int fd[2];

        if(pipe(fd) < 0) 
        {
            perror("Failed at pipe()");
            close(sock_server);
            return -1;
        }

        pid = fork();

        if(pid == -1)
        {
            perror("Failed at fork()");
            return -1;
            
        }

        if(pid > 0) {
            // Parent process which will only listen and accept new connections 
            close(sock_client);
            continue; // gets back to listening, continues with next client
        }

        else 
        {
            if(pid == 0)
            {
                 // Child process which will communicate with the client
                printf("Child : pid %d , ppid %d \n", getpid(), getppid());
                close(sock_server);

                // Each client executes its own connection for the database

                MYSQL * con = connectToDatabase();

                // Communicating with the client

                char message[100];
                char answer[1000];
                char buffer[100];
                char username[15];
                int length;  
                message[strlen(message)]='\0';

                while(1) // reading data written by the client
                {   
                    receiveCommand(message, sock_client, sizeof(message));


                // Aici fac toata procesarea - functia processCommand

                    processCommand(answer, message, sock_client, con, fd);

                // close(sock_client);
                
                }
                _exit(2); // child process is killed so SIGCHLD gets sent to the main process
                
             }
        }
         
    }
  

    close(sock_server);
    return 0;

    
}