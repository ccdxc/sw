/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl, CustomFormGroup } from '../../../utils/validators';
import { BaseModel, PropInfoItem } from '../basemodel/base-model';

import { SecurityPropagationStatus, ISecurityPropagationStatus } from './security-propagation-status.model';
import { SecuritySGRuleStatus, ISecuritySGRuleStatus } from './security-sg-rule-status.model';

export interface ISecurityNetworkSecurityPolicyStatus {
    'propagation-status'?: ISecurityPropagationStatus;
    'rule-status'?: Array<ISecuritySGRuleStatus>;
}


export class SecurityNetworkSecurityPolicyStatus extends BaseModel implements ISecurityNetworkSecurityPolicyStatus {
    /** The status of the configuration propagation to the Naples */
    'propagation-status': SecurityPropagationStatus = null;
    'rule-status': Array<SecuritySGRuleStatus> = null;
    public static propInfo: { [prop in keyof ISecurityNetworkSecurityPolicyStatus]: PropInfoItem } = {
        'propagation-status': {
            description:  'The status of the configuration propagation to the Naples',
            required: false,
            type: 'object'
        },
        'rule-status': {
            required: false,
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return SecurityNetworkSecurityPolicyStatus.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return SecurityNetworkSecurityPolicyStatus.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (SecurityNetworkSecurityPolicyStatus.propInfo[prop] != null &&
                        SecurityNetworkSecurityPolicyStatus.propInfo[prop].default != null);
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any, setDefaults:boolean = true) {
        super();
        this['propagation-status'] = new SecurityPropagationStatus();
        this['rule-status'] = new Array<SecuritySGRuleStatus>();
        this._inputValue = values;
        this.setValues(values, setDefaults);
    }

    /**
     * set the values for both the Model and the Form Group. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any, fillDefaults = true): void {
        if (values) {
            this['propagation-status'].setValues(values['propagation-status'], fillDefaults);
        } else {
            this['propagation-status'].setValues(null, fillDefaults);
        }
        if (values) {
            this.fillModelArray<SecuritySGRuleStatus>(this, 'rule-status', values['rule-status'], SecuritySGRuleStatus);
        } else {
            this['rule-status'] = [];
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'propagation-status': CustomFormGroup(this['propagation-status'].$formGroup, SecurityNetworkSecurityPolicyStatus.propInfo['propagation-status'].required),
                'rule-status': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<SecuritySGRuleStatus>('rule-status', this['rule-status'], SecuritySGRuleStatus);
            // We force recalculation of controls under a form group
            Object.keys((this._formGroup.get('propagation-status') as FormGroup).controls).forEach(field => {
                const control = this._formGroup.get('propagation-status').get(field);
                control.updateValueAndValidity();
            });
            // We force recalculation of controls under a form group
            Object.keys((this._formGroup.get('rule-status') as FormGroup).controls).forEach(field => {
                const control = this._formGroup.get('rule-status').get(field);
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
            this['propagation-status'].setFormGroupValuesToBeModelValues();
            this.fillModelArray<SecuritySGRuleStatus>(this, 'rule-status', this['rule-status'], SecuritySGRuleStatus);
        }
    }
}

