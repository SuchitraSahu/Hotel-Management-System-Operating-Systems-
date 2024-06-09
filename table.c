#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/types.h>

int main(){
	//Declaration of Initial variables
	int tablenumber=0,numofcustomer=0;
	//**This is Shared Memory Creation**  SM[0]= this byte is to facilitate Synchronisation,SM[1]=This byte is to look out for desynchronisation,SM[2]=Error byte SM[3]-SM[103]=is for data transfer.
	printf("Enter Table Number:");
	scanf("%d",&tablenumber);
	fflush(stdin); //flush output buffer to stream (temp buff)
	
	key_t key =ftok("menu.txt",tablenumber);
	if (key == -1) {
        	printf("ftok falliure");
        	return -1;
    	}
    	int shmid = shmget(key, 1000*sizeof(int), 0666);
	if (shmid == -1) {
        	printf("shmget falliure");
        	return -1;
    	}
    	int *shared_memory = (int *)shmat(shmid, NULL, 0); //r&w
	if (shared_memory == (int *)-1) {
        	printf("shmat falliure");
        	return -1;
    	}
	//**This is Shared Memory Creation**

	//This is the Main LOOP
	while(1){
	
		int ifparent=1,ifchild=-1;
	
		//Scan Customers 
		if(numofcustomer==0){
			printf("Enter Number of Customers at Table (maximum no.of customers can be 5):");
			scanf("%d",&numofcustomer);
			fflush(stdin);
		}
		while(*shared_memory!=9){}//Wait for Waiter//2 syn with waiter 
		
		shared_memory[2]=numofcustomer;
		*(shared_memory)=10;//3 sending num of customer to waiter
		
		//Print the contents of a file
		FILE* ptr = fopen("menu.txt", "r");

    		if (ptr == NULL) {
        		perror("Error opening the file");
        		exit(EXIT_FAILURE);
    		}

    		char* line = NULL;
    		size_t len = 0;

    		while (getline(&line, &len, ptr) != -1) {
        		printf("%s", line);
    		}
		fclose(ptr);
    		free(line);
    		//fclose(ptr);
	
		pid_t customerPid[numofcustomer];//pid array to track customerspid
		int fd[numofcustomer][2]; //aray for pipes for communication with table and cust
	
		
	
		
		int incorrectorder=1;
		while(incorrectorder){
		int customerincorrect[5]={0,0,0,0,0};//track incorrect order
		shared_memory[1]=0; // reset order process now
		for(int i=0;i<numofcustomer;i++){
			pipe(fd[i]); //create pipe for each cust process
			if(ifparent){
			
			
				customerPid[i]=fork(); //create cust process
				if(customerPid[i]==0){
					ifparent=0;
					ifchild=i;
					break;
				}else{
				close(fd[i][1]); //close write end pipe
				wait(NULL);

				int array[100];
				for(int j=0;j<100;j++){
					array[j]=0;
				}
				
				for(int j=0;j<100;j++){
					read(fd[i][0],&array[j],sizeof(array[0]));//sizeof(array[0])/sizeof(array[0][0])
				}
			
			
				while(*shared_memory!=9){}//6
				for(int j=0;j<100;j++){
					*(shared_memory+j+3)=array[j]; //copy process
				}
				*(shared_memory+1)=i; // curr cust
				*(shared_memory+2)=0; //urike dola
				
				*(shared_memory)=10;//7 //notify waiter 
				
				while(*shared_memory!=9){}//10wait for answer
			
			
				
			
				customerincorrect[i]=shared_memory[2]; //track incorrect
				
				
			
				}
			
			}
		
		}
		
		if(ifparent==1){
		//12
		incorrectorder=0;
		
		for(int j=0;j<5;j++){
			incorrectorder=customerincorrect[j]+incorrectorder;
		}
		if(incorrectorder){
			printf("Incorrect order given.");
		}
		}else{break;}
		}
	
	
	
	
		//bill calc
		if(ifparent==1){
			*shared_memory=10;
			//index 0 is for sync
			//index 1 bill anduke
			while(shared_memory[0]!=9){} // waits until waiter signals bil done 
			int total=shared_memory[1]; //retreives total bill calculated by waiter
			shared_memory[1]=0; //reset
			printf("The total bill amount is %d INR.\n",total);
			
		
			printf("Will there be more customers? No = -1 \n");
			int input=0;
			scanf("%d",&input);
			if(input==-1){
				shared_memory[2]=input;
				shared_memory[0]=10;
				shmdt(shared_memory); //detach shared memory
				return 0;
			}else{
				numofcustomer=input;
				shared_memory[2]=input;
				shared_memory[1]=0;
				shared_memory[0]=10;
			}
		
			}else{
				close(fd[ifchild][0]); //closing pipe
				printf("\nEnter the serial number(s) of the item(s) to order from the menu. Enter -1 when done:\n");
				fflush(stdout);
				int order[100]={0};
				int input1=1,input2,input3;//2,3,0,0

			while(input1!=-1){ //for serial no. //2 for quan
				sleep(1); //delay
				scanf("%d",&input1);
				fflush(stdin);			
			
				if(input1==-1){
					break;
				}else{
					scanf("%d",&input2);//3->order[2]=input2;
					order[input1-1]=input2;
				}
			
			}
		
			//print order and write to pipe
			for(int i=0;i<4;i++){printf("%d:%d;\n",i+1,order[i]); }
			char msg[4];
			for(int i=0;i<4;i++){
				msg[i]=48+order[i];
			} 
			write(fd[ifchild][1],order,sizeof(order));
		
			return 0;
		
			}
	
	}
	
	
}
