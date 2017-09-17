// Pipeline Processor Simulator
// 7873 MAPS
// Sean Shortreed 2017
// DFD created in Word and exported as PDF

package main

// Imported packages

import (
	"fmt"       // for console I/O
	"math/rand" // for randomly creating opcodes
	"time"      // for the random number generator and 'executing' opcodes
)

//////////////////////////////////////////////////////////////////////////////////
// const definitions
//////////////////////////////////////////////////////////////////////////////////

const opcodeLimit = 5 // program will generate instructions between 1 and 5
const pipelines = 3   // there will be 3 pipelines

//////////////////////////////////////////////////////////////////////////////////
// Function definitions
//////////////////////////////////////////////////////////////////////////////////

func generateInstructionsRandomly(instruction chan<- int) {
	//for keeps things running forever
	for {
		opcode := (rand.Intn(opcodeLimit) + 1)                 //generate random opcode
		fmt.Printf("Generating Instruction - %d - \n", opcode) //print to screen
		instruction <- opcode                                  //pass to dispatcher
	}
}

func dispatcher(instruction <-chan int, toPipeline [pipelines]chan int, readyForNext [pipelines]chan bool) {
	sequenceNumber := 10 //initialise the sequenceNumber, used to order the opcodes to retire in order

	for { //for keeps it running forever
		opcode := <-instruction //get various data required
		opcodePlus := opcode + sequenceNumber
		sequenceNumber = sequenceNumber + 10

		select { //pass the opcode to the first available pipeline
		case <-readyForNext[0]:
			toPipeline[0] <- opcodePlus
			fmt.Printf("				Sending - %d - to Pipeline A\n", opcode)
		case <-readyForNext[1]:
			toPipeline[1] <- opcodePlus
			fmt.Printf("				Sending - %d - to Pipeline B\n", opcode)
		case <-readyForNext[2]:
			toPipeline[2] <- opcodePlus
			fmt.Printf("				Sending - %d - to Pipeline C\n", opcode)
		}
	}
}

func pipeline(fromDispatcher <-chan int, retireIt chan<- int, readyForNext chan<- bool) {
	for {
		readyForNext <- true //inform dispatcher to send it a new opcode
		opcodePlus := <-fromDispatcher
		seconds := opcodePlus % 10
		time.Sleep(time.Duration(seconds) * time.Second) //sleep for required amount of time
		retireIt <- opcodePlus                           //pass opcode to retirement
	}
}

func retireInstruction(fromPipeline [pipelines]chan int) {
	//sortStages = opcodeLimit
	//create opcodes
	var opcodes [opcodeLimit + 1]chan int //create a new array of channels to sort retired opcodes
	for i := range opcodes {
		opcodes[i] = make(chan int)
	}

	//start various parts of the sort and retire section in paralell
	go pushToSorter(fromPipeline, opcodes)
	go repeatSortIntoOrder(opcodes)
	go showRetiredOpcode(opcodes)
}

func pushToSorter(fromPipeline [pipelines]chan int, opcodes [opcodeLimit + 1]chan int) {
	for {
		select { //take opcode from whichever pipeline and send it into the sorter
		case opcodePlus := <-fromPipeline[0]:
			opcodes[0] <- opcodePlus
			fmt.Printf("								Completed opcode - %d -\n", opcodePlus%10)
		case opcodePlus := <-fromPipeline[1]:
			opcodes[0] <- opcodePlus
			fmt.Printf("								Completed opcode - %d -\n", opcodePlus%10)
		case opcodePlus := <-fromPipeline[2]:
			opcodes[0] <- opcodePlus
			fmt.Printf("								Completed opcode - %d -\n", opcodePlus%10)
		}
	}
}

func showRetiredOpcode(opcodes [opcodeLimit + 1]chan int) {
	for { //print out (and thus retire) any opcodes sent
		inOrderOpcodePlus := <-opcodes[opcodeLimit]
		fmt.Printf("												Retiring opcode - %d -\n", inOrderOpcodePlus%10)
	}
}

func sortIntoOrder(opcodeIn, opcodeOut chan int) {
	//check if opcode is larger (using the tag) than the next, if so pass it on, else take next
	currentOpcodePlus := <-opcodeIn
	for {
		opcodePlus := <-opcodeIn
		if (opcodePlus / 10) > (currentOpcodePlus / 10) {
			opcodeOut <- currentOpcodePlus
			currentOpcodePlus = opcodePlus
		} else {
			opcodeOut <- opcodePlus
		}
	}
}

func repeatSortIntoOrder(opcodes [opcodeLimit + 1]chan int) {
	//run the entire sort process in parallel
	go sortIntoOrder(opcodes[0], opcodes[1])
	go sortIntoOrder(opcodes[1], opcodes[2])
	go sortIntoOrder(opcodes[2], opcodes[3])
	go sortIntoOrder(opcodes[3], opcodes[4])
	go sortIntoOrder(opcodes[4], opcodes[5])
}

//////////////////////////////////////////////////////////////////////////////////
//  Main program, create required channels, then start goroutines in parallel.
//////////////////////////////////////////////////////////////////////////////////

func main() {
	rand.Seed(time.Now().Unix()) // Seed the random number generator

	//Create channels -----------------------
	var toPipeline [pipelines]chan int
	var fromPipeline [pipelines]chan int
	for i := range toPipeline {
		toPipeline[i] = make(chan int)
		fromPipeline[i] = make(chan int)
	}

	var readyForNext [pipelines]chan bool
	for i := range readyForNext {
		readyForNext[i] = make(chan bool)
	}

	instruction := make(chan int)
	//---------------------------------------

	//Set up screen
	fmt.Printf("--------\n")
	fmt.Printf("Pipeline Processor Simulator, Based of of A.Oram's OCC file.\n")
	fmt.Printf("Basic Simulation with In Order sorted opcodes. Does not end gracefully. S Shortreed 2017.\n")
	fmt.Printf("--------\n")

	//Start go routines
	go generateInstructionsRandomly(instruction)
	go dispatcher(instruction, toPipeline, readyForNext)
	//PAR for piplines
	for i := range fromPipeline {
		go pipeline(toPipeline[i], fromPipeline[i], readyForNext[i])
	}
	go retireInstruction(fromPipeline)

	for { // Needed to keep the 'main' process alive!
	}

} // end of main /////////////////////////////////////////////////////////////////
