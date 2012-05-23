#include <iostream>
#include <string>

namespace consumer
{
   struct pipe_creation_failed {};
   struct fork_failed {};
   struct consumer_launch_failed {};

   class Consumer
   {
      // file descriptor for writing to the consumer
      private: int fd_write;

      public: Consumer()
         :
         fd_write(0)
      {
         static const int READ  = 0;
         static const int WRITE = 1;

         // stores the read and write file descriptors for a pipe
         int fd[2] = {0};

         // attempt to create a pipe...
         if(pipe(fd) == -1)
         {
            // ...and inform the caller if it was unsuccessful
            throw pipe_creation_failed();
         }

         pid_t pid(fork());

         // forked child process see pid as zero
         if(pid == 0)
         {
            // the consumer doesn't need to write to the pipe
            close(fd[WRITE]);

            // launch the application 'consumer' in the current working directory and pass it the pipe file descriptor for the 'read' end
            execl("consumer", "consumer", std::to_string(fd[READ]).c_str(), static_cast<char *>(0));
            
            // if execl() returned, an error occurred
            throw consumer_launch_failed();
         }

         // the producer doesn't need to read from the pipe
         close(fd[READ]);

         // fork failed
         if(pid < 0)
         {
            // nobody to write to
            close(fd[WRITE]);

            // notify the caller            
            throw fork_failed();
         }

         // the send() function needs this for writing and the destructor closes this upon program termination
         fd_write = fd[1];
      }

      public: ~Consumer()
      {
         // close the write descriptor if it's open
         if(fd_write) close(fd_write);
      }

      public: void send(std::string msg)
      {
         // copy the incoming message length so we can access it as an lvalue reference
         unsigned len(msg.length());

         // create the output string with enough space to contain the message size + the message
         std::string out(msg.length() + sizeof(unsigned), '\0');
         
         // set the size to zero so we can append some data
         out.clear();
         
         // append the raw length
         out.append(reinterpret_cast<char const *>(&len), sizeof(unsigned));
         
         // append the incoming message
         out.append(msg);

         // write the whole message to the consumer
         write(fd_write, out.c_str(), out.length());
      }
   };

   static void send(std::string message)
   {
      // on first call to this method, the consumer is launched
      static Consumer client;

      // send the message to the consumer
      client.send(message);
   }
}

auto main(int argc, char *argv[]) -> int
{
   try
   {
      // send each argument to the consumer
      for(int i(0); i < argc; ++i) consumer::send(argv[i]);
      
      // success!
      return 0;
   }
   catch(consumer::pipe_creation_failed const &)
   {
      std::cout << "[ERROR] Failed to create the pipe" << std::endl;
      return 1;
   }
   catch(consumer::fork_failed const &)
   {
      std::cout << "[ERROR] Failed to fork" << std::endl;
      return 2;
   }
   catch(consumer::consumer_launch_failed const &)
   {
      std::cout << "[ERROR] Failed to launch the consumer (does the file exist, is it executable?)" << std::endl;
      return 3;
   }
}
