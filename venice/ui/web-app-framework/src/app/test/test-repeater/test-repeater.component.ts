import { Component, OnInit, ViewEncapsulation } from '@angular/core';
import { FormArray, FormControl, FormGroup } from '@angular/forms';
import { RepeaterData, ValueType } from '../../../public_api';

@Component({
  selector: 'app-test-repeater',
  templateUrl: './test-repeater.component.html',
  styleUrls: ['./test-repeater.component.css'],
  encapsulation: ViewEncapsulation.None
})
export class TestRepeaterComponent implements OnInit {
  data: RepeaterData[] = [];
  output: any;
  formGroup = new FormGroup({
    test: new FormArray([
      new FormControl({ keyFormControl: 'name', operatorFormControl: 'is not', valueFormControl: 'testing' }),
      new FormControl({ keyFormControl: 'severity', operatorFormControl: 'is', valueFormControl: 'critical' })
    ])
  });

  constructor() { }

  ngOnInit() {
    this.data = [
      {
        key: { label: 'severity', value: 'severity' },
        operators: [
          { label: 'is', value: 'is' },
          { label: 'is', value: 'not' }
        ],
        values: [
          { label: 'critical', value: 'critical' },
          { label: 'warning', value: 'warning' },
          { label: 'info', value: 'info' },
        ],
        valueType: ValueType.singleSelect
      },
      {
        key: { label: 'kind', value: 'kind' },
        operators: [
          { label: 'is', value: 'is' },
          { label: 'is not', value: 'is not' },
        ],
        values: [
          { label: 'Node', value: 'Node' },
          { label: 'Network', value: 'Network' },
          { label: 'Endpoint', value: 'Endpoint' },
        ],
        valueType: ValueType.multiSelect
      },
      {
        key: { label: 'name', value: 'name' },
        operators: [
          { label: 'is', value: 'is' },
          { label: 'is not', value: 'is not' },
        ],
        valueType: ValueType.inputField
      },
    ]
  }

  handleRepeaterData(values) {
    this.output = values;
  }

  repeaterOutput() {
    console.log(this.output);
  }

  formControlOutput() {
    console.log(this.formGroup.value);
  }
}
