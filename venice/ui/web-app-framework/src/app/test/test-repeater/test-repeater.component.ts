import { Component, OnInit, ViewEncapsulation, ViewChild } from '@angular/core';
import { FormArray, FormControl, FormGroup } from '@angular/forms';
import { RepeaterData, ValueType, RepeaterComponent,  RepeaterItem } from '../../../public_api';

@Component({
  selector: 'app-test-repeater',
  templateUrl: './test-repeater.component.html',
  styleUrls: ['./test-repeater.component.css'],
  encapsulation: ViewEncapsulation.None
})
export class TestRepeaterComponent implements OnInit {
  data: RepeaterData[] = [];
  data2: RepeaterData[] = [];
  data3: RepeaterData[] = [];
  output: any;
  output2: any;
  output3: any;
  outputDyn: any;

  isToSelectField = true;

  formGroup = new FormGroup({
    test: new FormArray([
      new FormControl({ keyFormControl: 'name', operatorFormControl: 'is not', valueFormControl: 'testing' }),
      new FormControl({ keyFormControl: 'severity', operatorFormControl: 'is', valueFormControl: 'critical' })
    ]),
    test2: new FormArray([
      new FormControl({ keyFormControl: 'text', operatorFormControl: 'is not',
                        valueFormControl: 'testing1', keytextFormName: 'k1' }),
      new FormControl({ keyFormControl: 'text', operatorFormControl: 'is',
                        valueFormControl: 'critical', keytextFormName: 'k2'  })
    ])
  });

  @ViewChild('repeater2') repeater2: RepeaterComponent;

  constructor() { }

  ngOnInit() {
    this.data2 = [
      // must have only one object.
      {
        key: { label: 'text', value: 'text' },
        operators: [
          { label: 'is', value: 'is' },
          { label: 'is not', value: 'is not' },
        ],
        fieldType: ValueType.inputField,
        valueType: ValueType.inputField
      }
    ];

    this.data3 = [
      // must have only one object.
      {
        key: { label: 'text', value: 'text' },
        operators: [
          { label: 'is', value: 'is' },
          { label: 'is not', value: 'is not' },
        ],
        fieldType: ValueType.inputField,
        valueType: ValueType.inputField
      }
    ];

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
      }
    ];
  }

  handleRepeaterData(values) {
    this.output = values;
  }

  handleRepeater2Data(values) {
    this.output2 = values;
  }

  handleRepeater3Data(values) {
    this.output3 = values;
  }

  handleRepeaterDynData(values) {
    this.outputDyn = values;
  }

  repeaterOutput() {
    console.log(this.output);
  }

  /**
   * Test to printout repeater output.
   */
  repeater2Output() {
    console.log(this.output2);
    const values = this.repeater2.getValues();
    const testRepeaterValueItem = values[0];
    console.log(values);
  }

  repeater3Output() {
    console.log(this.output3);
  }

  repeaterDynOutput() {
    console.log(this.outputDyn);
  }

  formControlOutput() {
    console.log(this.formGroup.value);
  }

  getDynRepeaterData(): any {
    return this.getFieldData();
  }

  getFieldData(): any {
    const modelData = {
      'meta': [
        'name',
        'tenant'
      ],
      'spec': [
        'quorum-nodes',
        'virtual-ip'
      ],
      'status': [
        'leader',
        'build-date'
      ]
    };
    let fieldData = [];
    const keys = Object.keys(modelData);
    keys.filter((key) => {
        const keyData = this.getFieldDataHelper(modelData, key);
        fieldData = fieldData.concat(keyData);
    });


    return fieldData;
  }

  getFieldDataHelper(modelData: any, key: string): any[] {
    const fieldData = [];
    modelData[key].filter((item) => {
      if (this.acceptField(key, item)) {
        const config = {
          key: {
            label: key + '.' + item,
            value:  key + '.' + item,
          },
          operators: this.getFieldOperators(key, item),
          valueType: ValueType.inputField
        };
        fieldData.push(config);
      }
    });
    return fieldData;
  }

  acceptField(key, item): boolean {
    if (key === 'meta') {
      if (item === 'labels' || item === 'self-link') {
        return false;
      }
    }
    return true;
  }

  getFieldOperators(key, item): any[] {
    return [
      { label: 'equal', value: 'equal' },
      { label: 'not equal', value: 'not equal' },
      { label: 'in', value: 'in' },
      { label: 'not in', value: 'not in' }
    ];
  }

  /**
   * Customized placeholder for key
   * @param repeater
   * @param keyFormName
   */
  buildKeyPlaceholder(repeater: RepeaterItem, keyFormName: string): string {
    return 'customized key pl:' + repeater.formGroup.value[keyFormName];
  }

  /**
   * Customized placeholder value
   * @param repeater
   * @param keyFormName
   */
  buildValuePlaceholder(repeater: RepeaterItem, keyFormName: string): string {
    return 'Customized value pl:' + repeater.formGroup.value[keyFormName];
  }

  /**
   * This API serves html template
   */
  setShowSelectField(event) {
    this.isToSelectField = !this.isToSelectField;
  }
}
