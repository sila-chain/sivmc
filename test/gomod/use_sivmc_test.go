package sivmc_use

import (
	"github.com/sila-chain/sivmc/v12/bindings/go/sivmc"
	"testing"
)

var exampleVmPath = "./example-vm.so"

func TestGetVmName(t *testing.T) {
	vm, err := sivmc.Load(exampleVmPath)
	if err != nil {
		t.Fatalf("%v", err)
	}

	expectedName := "example_vm"
	if name := vm.Name(); name != expectedName {
		t.Errorf("wrong VM name: %s, expected %s", name, expectedName)
	}
}
