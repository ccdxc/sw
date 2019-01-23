/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { RolloutRolloutSpec_strategy,  RolloutRolloutSpec_strategy_uihint  } from './enums';
import { LabelsSelector, ILabelsSelector } from './labels-selector.model';
import { RolloutRolloutSpec_upgrade_type,  RolloutRolloutSpec_upgrade_type_uihint  } from './enums';

export interface IRolloutRolloutSpec {
    'version'?: string;
    'scheduled-start-time'?: Date;
    'duration'?: string;
    'strategy': RolloutRolloutSpec_strategy;
    'max-parallel'?: number;
    'max-nic-failures-before-abort'?: number;
    'order-constraints'?: Array<ILabelsSelector>;
    'suspend'?: boolean;
    'smartnics-only'?: boolean;
    'smartnic-must-match-constraint'?: boolean;
    'upgrade-type': RolloutRolloutSpec_upgrade_type;
}


export class RolloutRolloutSpec extends BaseModel implements IRolloutRolloutSpec {
    'version': string = null;
    'scheduled-start-time': Date = null;
    /** should be a valid time duration
     */
    'duration': string = null;
    'strategy': RolloutRolloutSpec_strategy = null;
    /** MaxParallel is the maximum number of nodes getting updated at any time
    This setting is applicable only to SmartNICs.
    Controller nodes are always upgraded one after another. */
    'max-parallel': number = null;
    'max-nic-failures-before-abort': number = null;
    'order-constraints': Array<LabelsSelector> = null;
    /** When Set to true, the current rollout is suspended. Existing Nodes/Services/SmartNICs in the middle of rollout continue
     rollout execution but any Nodes/Services/SmartNICs which has not started Rollout will not be scheduled one. */
    'suspend': boolean = null;
    'smartnics-only': boolean = null;
    'smartnic-must-match-constraint': boolean = null;
    'upgrade-type': RolloutRolloutSpec_upgrade_type = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'version': {
            type: 'string'
        },
        'scheduled-start-time': {
            type: 'Date'
        },
        'duration': {
            description:  'should be a valid time duration ',
            hint:  '2h',
            type: 'string'
        },
        'strategy': {
            enum: RolloutRolloutSpec_strategy_uihint,
            default: 'LINEAR',
            type: 'string'
        },
        'max-parallel': {
            description:  'MaxParallel is the maximum number of nodes getting updated at any time This setting is applicable only to SmartNICs. Controller nodes are always upgraded one after another.',
            type: 'number'
        },
        'max-nic-failures-before-abort': {
            type: 'number'
        },
        'order-constraints': {
            type: 'object'
        },
        'suspend': {
            description:  'When Set to true, the current rollout is suspended. Existing Nodes/Services/SmartNICs in the middle of rollout continue  rollout execution but any Nodes/Services/SmartNICs which has not started Rollout will not be scheduled one.',
            type: 'boolean'
        },
        'smartnics-only': {
            type: 'boolean'
        },
        'smartnic-must-match-constraint': {
            type: 'boolean'
        },
        'upgrade-type': {
            enum: RolloutRolloutSpec_upgrade_type_uihint,
            default: 'Disruptive',
            type: 'string'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return RolloutRolloutSpec.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return RolloutRolloutSpec.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (RolloutRolloutSpec.propInfo[prop] != null &&
                        RolloutRolloutSpec.propInfo[prop].default != null &&
                        RolloutRolloutSpec.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['order-constraints'] = new Array<LabelsSelector>();
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['version'] != null) {
            this['version'] = values['version'];
        } else if (fillDefaults && RolloutRolloutSpec.hasDefaultValue('version')) {
            this['version'] = RolloutRolloutSpec.propInfo['version'].default;
        } else {
            this['version'] = null
        }
        if (values && values['scheduled-start-time'] != null) {
            this['scheduled-start-time'] = values['scheduled-start-time'];
        } else if (fillDefaults && RolloutRolloutSpec.hasDefaultValue('scheduled-start-time')) {
            this['scheduled-start-time'] = RolloutRolloutSpec.propInfo['scheduled-start-time'].default;
        } else {
            this['scheduled-start-time'] = null
        }
        if (values && values['duration'] != null) {
            this['duration'] = values['duration'];
        } else if (fillDefaults && RolloutRolloutSpec.hasDefaultValue('duration')) {
            this['duration'] = RolloutRolloutSpec.propInfo['duration'].default;
        } else {
            this['duration'] = null
        }
        if (values && values['strategy'] != null) {
            this['strategy'] = values['strategy'];
        } else if (fillDefaults && RolloutRolloutSpec.hasDefaultValue('strategy')) {
            this['strategy'] = <RolloutRolloutSpec_strategy>  RolloutRolloutSpec.propInfo['strategy'].default;
        } else {
            this['strategy'] = null
        }
        if (values && values['max-parallel'] != null) {
            this['max-parallel'] = values['max-parallel'];
        } else if (fillDefaults && RolloutRolloutSpec.hasDefaultValue('max-parallel')) {
            this['max-parallel'] = RolloutRolloutSpec.propInfo['max-parallel'].default;
        } else {
            this['max-parallel'] = null
        }
        if (values && values['max-nic-failures-before-abort'] != null) {
            this['max-nic-failures-before-abort'] = values['max-nic-failures-before-abort'];
        } else if (fillDefaults && RolloutRolloutSpec.hasDefaultValue('max-nic-failures-before-abort')) {
            this['max-nic-failures-before-abort'] = RolloutRolloutSpec.propInfo['max-nic-failures-before-abort'].default;
        } else {
            this['max-nic-failures-before-abort'] = null
        }
        if (values) {
            this.fillModelArray<LabelsSelector>(this, 'order-constraints', values['order-constraints'], LabelsSelector);
        } else {
            this['order-constraints'] = [];
        }
        if (values && values['suspend'] != null) {
            this['suspend'] = values['suspend'];
        } else if (fillDefaults && RolloutRolloutSpec.hasDefaultValue('suspend')) {
            this['suspend'] = RolloutRolloutSpec.propInfo['suspend'].default;
        } else {
            this['suspend'] = null
        }
        if (values && values['smartnics-only'] != null) {
            this['smartnics-only'] = values['smartnics-only'];
        } else if (fillDefaults && RolloutRolloutSpec.hasDefaultValue('smartnics-only')) {
            this['smartnics-only'] = RolloutRolloutSpec.propInfo['smartnics-only'].default;
        } else {
            this['smartnics-only'] = null
        }
        if (values && values['smartnic-must-match-constraint'] != null) {
            this['smartnic-must-match-constraint'] = values['smartnic-must-match-constraint'];
        } else if (fillDefaults && RolloutRolloutSpec.hasDefaultValue('smartnic-must-match-constraint')) {
            this['smartnic-must-match-constraint'] = RolloutRolloutSpec.propInfo['smartnic-must-match-constraint'].default;
        } else {
            this['smartnic-must-match-constraint'] = null
        }
        if (values && values['upgrade-type'] != null) {
            this['upgrade-type'] = values['upgrade-type'];
        } else if (fillDefaults && RolloutRolloutSpec.hasDefaultValue('upgrade-type')) {
            this['upgrade-type'] = <RolloutRolloutSpec_upgrade_type>  RolloutRolloutSpec.propInfo['upgrade-type'].default;
        } else {
            this['upgrade-type'] = null
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'version': new FormControl(this['version']),
                'scheduled-start-time': new FormControl(this['scheduled-start-time']),
                'duration': new FormControl(this['duration']),
                'strategy': new FormControl(this['strategy'], [required, enumValidator(RolloutRolloutSpec_strategy), ]),
                'max-parallel': new FormControl(this['max-parallel']),
                'max-nic-failures-before-abort': new FormControl(this['max-nic-failures-before-abort']),
                'order-constraints': new FormArray([]),
                'suspend': new FormControl(this['suspend']),
                'smartnics-only': new FormControl(this['smartnics-only']),
                'smartnic-must-match-constraint': new FormControl(this['smartnic-must-match-constraint']),
                'upgrade-type': new FormControl(this['upgrade-type'], [required, enumValidator(RolloutRolloutSpec_upgrade_type), ]),
            });
            // generate FormArray control elements
            this.fillFormArray<LabelsSelector>('order-constraints', this['order-constraints'], LabelsSelector);
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['version'].setValue(this['version']);
            this._formGroup.controls['scheduled-start-time'].setValue(this['scheduled-start-time']);
            this._formGroup.controls['duration'].setValue(this['duration']);
            this._formGroup.controls['strategy'].setValue(this['strategy']);
            this._formGroup.controls['max-parallel'].setValue(this['max-parallel']);
            this._formGroup.controls['max-nic-failures-before-abort'].setValue(this['max-nic-failures-before-abort']);
            this.fillModelArray<LabelsSelector>(this, 'order-constraints', this['order-constraints'], LabelsSelector);
            this._formGroup.controls['suspend'].setValue(this['suspend']);
            this._formGroup.controls['smartnics-only'].setValue(this['smartnics-only']);
            this._formGroup.controls['smartnic-must-match-constraint'].setValue(this['smartnic-must-match-constraint']);
            this._formGroup.controls['upgrade-type'].setValue(this['upgrade-type']);
        }
    }
}

