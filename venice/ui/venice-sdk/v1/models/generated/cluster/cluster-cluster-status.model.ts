/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl, CustomFormGroup } from '../../../utils/validators';
import { BaseModel, PropInfoItem } from '../basemodel/base-model';

import { ClusterClusterCondition, IClusterClusterCondition } from './cluster-cluster-condition.model';
import { ClusterQuorumStatus, IClusterQuorumStatus } from './cluster-quorum-status.model';

export interface IClusterClusterStatus {
    'leader'?: string;
    'last-leader-transition-time'?: Date;
    'auth-bootstrapped'?: boolean;
    'conditions'?: Array<IClusterClusterCondition>;
    'quorum-status'?: IClusterQuorumStatus;
}


export class ClusterClusterStatus extends BaseModel implements IClusterClusterStatus {
    /** Leader contains the node name of the cluster leader. */
    'leader': string = null;
    /** LastLeaderTransitionTime is when the leadership changed last time */
    'last-leader-transition-time': Date = null;
    /** AuthBootstrapped indicates whether the Cluster has Completed BootStrap of Auth */
    'auth-bootstrapped': boolean = null;
    /** List of current cluster conditions */
    'conditions': Array<ClusterClusterCondition> = null;
    /** Quorum represents the current state of the quorum, including registered members and health */
    'quorum-status': ClusterQuorumStatus = null;
    public static propInfo: { [prop in keyof IClusterClusterStatus]: PropInfoItem } = {
        'leader': {
            description:  'Leader contains the node name of the cluster leader.',
            required: false,
            type: 'string'
        },
        'last-leader-transition-time': {
            description:  'LastLeaderTransitionTime is when the leadership changed last time',
            required: false,
            type: 'Date'
        },
        'auth-bootstrapped': {
            description:  'AuthBootstrapped indicates whether the Cluster has Completed BootStrap of Auth',
            required: false,
            type: 'boolean'
        },
        'conditions': {
            description:  'List of current cluster conditions',
            required: false,
            type: 'object'
        },
        'quorum-status': {
            description:  'Quorum represents the current state of the quorum, including registered members and health',
            required: false,
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return ClusterClusterStatus.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return ClusterClusterStatus.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (ClusterClusterStatus.propInfo[prop] != null &&
                        ClusterClusterStatus.propInfo[prop].default != null);
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['conditions'] = new Array<ClusterClusterCondition>();
        this['quorum-status'] = new ClusterQuorumStatus();
        this._inputValue = values;
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values && values['leader'] != null) {
            this['leader'] = values['leader'];
        } else if (fillDefaults && ClusterClusterStatus.hasDefaultValue('leader')) {
            this['leader'] = ClusterClusterStatus.propInfo['leader'].default;
        } else {
            this['leader'] = null
        }
        if (values && values['last-leader-transition-time'] != null) {
            this['last-leader-transition-time'] = values['last-leader-transition-time'];
        } else if (fillDefaults && ClusterClusterStatus.hasDefaultValue('last-leader-transition-time')) {
            this['last-leader-transition-time'] = ClusterClusterStatus.propInfo['last-leader-transition-time'].default;
        } else {
            this['last-leader-transition-time'] = null
        }
        if (values && values['auth-bootstrapped'] != null) {
            this['auth-bootstrapped'] = values['auth-bootstrapped'];
        } else if (fillDefaults && ClusterClusterStatus.hasDefaultValue('auth-bootstrapped')) {
            this['auth-bootstrapped'] = ClusterClusterStatus.propInfo['auth-bootstrapped'].default;
        } else {
            this['auth-bootstrapped'] = null
        }
        if (values) {
            this.fillModelArray<ClusterClusterCondition>(this, 'conditions', values['conditions'], ClusterClusterCondition);
        } else {
            this['conditions'] = [];
        }
        if (values) {
            this['quorum-status'].setValues(values['quorum-status'], fillDefaults);
        } else {
            this['quorum-status'].setValues(null, fillDefaults);
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'leader': CustomFormControl(new FormControl(this['leader']), ClusterClusterStatus.propInfo['leader']),
                'last-leader-transition-time': CustomFormControl(new FormControl(this['last-leader-transition-time']), ClusterClusterStatus.propInfo['last-leader-transition-time']),
                'auth-bootstrapped': CustomFormControl(new FormControl(this['auth-bootstrapped']), ClusterClusterStatus.propInfo['auth-bootstrapped']),
                'conditions': new FormArray([]),
                'quorum-status': CustomFormGroup(this['quorum-status'].$formGroup, ClusterClusterStatus.propInfo['quorum-status'].required),
            });
            // generate FormArray control elements
            this.fillFormArray<ClusterClusterCondition>('conditions', this['conditions'], ClusterClusterCondition);
            // We force recalculation of controls under a form group
            Object.keys((this._formGroup.get('conditions') as FormGroup).controls).forEach(field => {
                const control = this._formGroup.get('conditions').get(field);
                control.updateValueAndValidity();
            });
            // We force recalculation of controls under a form group
            Object.keys((this._formGroup.get('quorum-status') as FormGroup).controls).forEach(field => {
                const control = this._formGroup.get('quorum-status').get(field);
                control.updateValueAndValidity();
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['leader'].setValue(this['leader']);
            this._formGroup.controls['last-leader-transition-time'].setValue(this['last-leader-transition-time']);
            this._formGroup.controls['auth-bootstrapped'].setValue(this['auth-bootstrapped']);
            this.fillModelArray<ClusterClusterCondition>(this, 'conditions', this['conditions'], ClusterClusterCondition);
            this['quorum-status'].setFormGroupValuesToBeModelValues();
        }
    }
}

