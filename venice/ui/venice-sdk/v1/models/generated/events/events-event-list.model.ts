/**
 * This file is generated by the SwaggerTSGenerator.
 * Do not edit.
*/
/* tslint:disable */
import { Validators, FormControl, FormGroup, FormArray, ValidatorFn } from '@angular/forms';
import { minValueValidator, maxValueValidator, enumValidator } from './validators';
import { BaseModel, PropInfoItem } from './base-model';

import { EventsEvent, IEventsEvent } from './events-event.model';

export interface IEventsEventList {
    'kind'?: string;
    'api-version'?: string;
    'resource-version'?: string;
    'items'?: Array<IEventsEvent>;
}


export class EventsEventList extends BaseModel implements IEventsEventList {
    'kind': string = null;
    'api-version': string = null;
    'resource-version': string = null;
    'items': Array<EventsEvent> = null;
    public static propInfo: { [prop: string]: PropInfoItem } = {
        'kind': {
            type: 'string'
        },
        'api-version': {
            type: 'string'
        },
        'resource-version': {
            type: 'string'
        },
        'items': {
            type: 'object'
        },
    }

    public getPropInfo(propName: string): PropInfoItem {
        return EventsEventList.propInfo[propName];
    }

    /**
     * Returns whether or not there is an enum property with a default value
    */
    public static hasDefaultValue(prop) {
        return (EventsEventList.propInfo[prop] != null &&
                        EventsEventList.propInfo[prop].default != null &&
                        EventsEventList.propInfo[prop].default != '');
    }

    /**
     * constructor
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    constructor(values?: any) {
        super();
        this['items'] = new Array<EventsEvent>();
        this.setValues(values);
    }

    /**
     * set the values. If a value isn't provided and we have a default, we use that.
     * @param values Can be used to set a webapi response to this newly constructed model
    */
    setValues(values: any): void {
        if (values && values['kind'] != null) {
            this['kind'] = values['kind'];
        } else if (EventsEventList.hasDefaultValue('kind')) {
            this['kind'] = EventsEventList.propInfo['kind'].default;
        }
        if (values && values['api-version'] != null) {
            this['api-version'] = values['api-version'];
        } else if (EventsEventList.hasDefaultValue('api-version')) {
            this['api-version'] = EventsEventList.propInfo['api-version'].default;
        }
        if (values && values['resource-version'] != null) {
            this['resource-version'] = values['resource-version'];
        } else if (EventsEventList.hasDefaultValue('resource-version')) {
            this['resource-version'] = EventsEventList.propInfo['resource-version'].default;
        }
        if (values) {
            this.fillModelArray<EventsEvent>(this, 'items', values['items'], EventsEvent);
        }
    }




    protected getFormGroup(): FormGroup {
        if (!this._formGroup) {
            this._formGroup = new FormGroup({
                'kind': new FormControl(this['kind']),
                'api-version': new FormControl(this['api-version']),
                'resource-version': new FormControl(this['resource-version']),
                'items': new FormArray([]),
            });
            // generate FormArray control elements
            this.fillFormArray<EventsEvent>('items', this['items'], EventsEvent);
        }
        return this._formGroup;
    }

    setFormGroupValues() {
        if (this._formGroup) {
            this._formGroup.controls['kind'].setValue(this['kind']);
            this._formGroup.controls['api-version'].setValue(this['api-version']);
            this._formGroup.controls['resource-version'].setValue(this['resource-version']);
            this.fillModelArray<EventsEvent>(this, 'items', this['items'], EventsEvent);
        }
    }
}

