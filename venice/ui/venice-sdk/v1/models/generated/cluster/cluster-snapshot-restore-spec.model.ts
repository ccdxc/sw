/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, minLengthValidator, maxLengthValidator, required, enumValidator, patternValidator, CustomFormControl, CustomFormGroup } from '../../../utils/validators';
import { BaseModel, PropInfoItem } from '../basemodel/base-model';


export interface IClusterSnapshotRestoreSpec {
    'path'?: string;
    '_ui'?: any;
}


export class ClusterSnapshotRestoreSpec extends BaseModel implements IClusterSnapshotRestoreSpec {
    /** Field for holding arbitrary ui state */
    '_ui': any = {};
    'path': string = null;
    public static propInfo: { [prop in keyof IClusterSnapshotRestoreSpec]: PropInfoItem } = {
        'path': {
            required: false,
            type: 'string'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return ClusterSnapshotRestoreSpec.propInfo[propName];
    }

    public getPropInfoConfig(): { [key:string]:PropInfoItem } {
        return ClusterSnapshotRestoreSpec.propInfo;
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (ClusterSnapshotRestoreSpec.propInfo[prop] != null &&
                        ClusterSnapshotRestoreSpec.propInfo[prop].default != null);
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
        if (values && values['_ui']) {
            this['_ui'] = values['_ui']
        }
        if (values && values['path'] != null) {
            this['path'] = values['path'];
        } else if (fillDefaults && ClusterSnapshotRestoreSpec.hasDefaultValue('path')) {
            this['path'] = ClusterSnapshotRestoreSpec.propInfo['path'].default;
        } else {
            this['path'] = null
        }
        this.setFormGroupValuesToBeModelValues();
    }


    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'path': CustomFormControl(new FormControl(this['path']), ClusterSnapshotRestoreSpec.propInfo['path']),
            });
        }
        return this._formGroup;
    }

    setModelToBeFormGroupValues() {
        this.setValues(this.$formGroup.value, false);
    }

    setFormGroupValuesToBeModelValues() {
        if (this._formGroup) {
            this._formGroup.controls['path'].setValue(this['path']);
        }
    }
}

