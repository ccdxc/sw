/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl, CustomFormGroup } from '../../../utils/validators';
import { BaseModel, PropInfoItem } from '../basemodel/base-model';

import { RolloutRolloutPhase_phase,  RolloutRolloutPhase_phase_uihint  } from './enums';

export interface IRolloutRolloutPhase {
    'name'?: string;
    'phase': RolloutRolloutPhase_phase;
    'start-time'?: Date;
    'end-time'?: Date;
    'reason'?: string;
    'message'?: string;
    'num-retries'?: number;
}


export class RolloutRolloutPhase extends BaseModel implements IRolloutRolloutPhase {
    /** Name of the Node, Service or DistributedServiceCard */
    'name': string = null;
    /** Phase indicates a certain rollout phase/condition */
    'phase': RolloutRolloutPhase_phase = null;
    /** The time of starting the rollout for this node/service. 
This does not include the pre-check which can happen way before the actual rollout. */
    'start-time': Date = null;
    /** Time at which rollout completed for this node/service */
    'end-time': Date = null;
    /** The reason for the Phase last transition, if any */
    'reason': string = null;
    /** A detailed message indicating details about the transition. */
    'message': string = null;
    /** Number of retries rollout performed. */
    'num-retries': number = null;
    public static propInfo: { [prop in keyof IRolloutRolloutPhase]: PropInfoItem } = {
        'name': {
            description:  'Name of the Node, Service or DistributedServiceCard',
            required: false,
            type: 'string'
        },
        'phase': {
            enum: RolloutRolloutPhase_phase_uihint,
            default: 'pre-check',
            description:  'Phase indicates a certain rollout phase/condition',
            required: true,
            type: 'string'
        },
        'start-time': {
            description:  'The time of starting the rollout for this node/service.  This does not include the pre-check which can happen way before the actual rollout.',
            required: false,
            type: 'Date'
        },
        'end-time': {
            description:  'Time at which rollout completed for this node/service',
            required: false,
            type: 'Date'
        },
        'reason': {
            description:  'The reason for the Phase last transition, if any',
            required: false,
            type: 'string'
        },
        'message': {
            description:  'A detailed message indicating details about the transition.',
            required: false,
            type: 'string'
        },
        'num-retries': {
            default: parseInt('0'),
            description:  'Number of retries rollout performed.',
            required: false,
            type: 'number'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return RolloutRolloutPhase.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return RolloutRolloutPhase.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (RolloutRolloutPhase.propInfo[prop] != null &&
                        RolloutRolloutPhase.propInfo[prop].default != null);
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this._inputValue = values;
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['name'] != null) {
            this['name'] = values['name'];
        } else if (fillDefaults && RolloutRolloutPhase.hasDefaultValue('name')) {
            this['name'] = RolloutRolloutPhase.propInfo['name'].default;
        } else {
            this['name'] = null
        }
        if (values && values['phase'] != null) {
            this['phase'] = values['phase'];
        } else if (fillDefaults && RolloutRolloutPhase.hasDefaultValue('phase')) {
            this['phase'] = <RolloutRolloutPhase_phase>  RolloutRolloutPhase.propInfo['phase'].default;
        } else {
            this['phase'] = null
        }
        if (values && values['start-time'] != null) {
            this['start-time'] = values['start-time'];
        } else if (fillDefaults && RolloutRolloutPhase.hasDefaultValue('start-time')) {
            this['start-time'] = RolloutRolloutPhase.propInfo['start-time'].default;
        } else {
            this['start-time'] = null
        }
        if (values && values['end-time'] != null) {
            this['end-time'] = values['end-time'];
        } else if (fillDefaults && RolloutRolloutPhase.hasDefaultValue('end-time')) {
            this['end-time'] = RolloutRolloutPhase.propInfo['end-time'].default;
        } else {
            this['end-time'] = null
        }
        if (values && values['reason'] != null) {
            this['reason'] = values['reason'];
        } else if (fillDefaults && RolloutRolloutPhase.hasDefaultValue('reason')) {
            this['reason'] = RolloutRolloutPhase.propInfo['reason'].default;
        } else {
            this['reason'] = null
        }
        if (values && values['message'] != null) {
            this['message'] = values['message'];
        } else if (fillDefaults && RolloutRolloutPhase.hasDefaultValue('message')) {
            this['message'] = RolloutRolloutPhase.propInfo['message'].default;
        } else {
            this['message'] = null
        }
        if (values && values['num-retries'] != null) {
            this['num-retries'] = values['num-retries'];
        } else if (fillDefaults && RolloutRolloutPhase.hasDefaultValue('num-retries')) {
            this['num-retries'] = RolloutRolloutPhase.propInfo['num-retries'].default;
        } else {
            this['num-retries'] = null
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'name': CustomFormControl(new FormControl(this['name']), RolloutRolloutPhase.propInfo['name']),
                'phase': CustomFormControl(new FormControl(this['phase'], [required, enumValidator(RolloutRolloutPhase_phase), ]), RolloutRolloutPhase.propInfo['phase']),
                'start-time': CustomFormControl(new FormControl(this['start-time']), RolloutRolloutPhase.propInfo['start-time']),
                'end-time': CustomFormControl(new FormControl(this['end-time']), RolloutRolloutPhase.propInfo['end-time']),
                'reason': CustomFormControl(new FormControl(this['reason']), RolloutRolloutPhase.propInfo['reason']),
                'message': CustomFormControl(new FormControl(this['message']), RolloutRolloutPhase.propInfo['message']),
                'num-retries': CustomFormControl(new FormControl(this['num-retries']), RolloutRolloutPhase.propInfo['num-retries']),
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['name'].setValue(this['name']);
            this._formGroup.controls['phase'].setValue(this['phase']);
            this._formGroup.controls['start-time'].setValue(this['start-time']);
            this._formGroup.controls['end-time'].setValue(this['end-time']);
            this._formGroup.controls['reason'].setValue(this['reason']);
            this._formGroup.controls['message'].setValue(this['message']);
            this._formGroup.controls['num-retries'].setValue(this['num-retries']);
        }
    }
}

