/**-----
 Angular imports
 ------------------*/
import { async, ComponentFixture, TestBed, inject } from '@angular/core/testing';
import { By } from '@angular/platform-browser';
import { Component } from '@angular/core';
import { DebugElement } from '@angular/core';
import { RouterTestingModule } from '@angular/router/testing';
import { NgModule } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { Observable } from 'rxjs/Observable';
import { combineAll } from 'rxjs/operator/combineAll';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';


import { MatIconRegistry } from '@angular/material';


/**-----
 Third-parties imports
 ------------------*/
import { MomentModule } from 'angular2-moment';
import { PrimengModule } from '@lib/primeng.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
/**-----
 Venice web-app imports
 ------------------*/
import { ControllerService } from '@app/services/controller.service';
import { AlerttableService } from '@app/services/alerttable.service';
import { SharedModule } from '@app/components/shared/shared.module';
import { AlertseventsComponent } from './alertsevents.component';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';

@Component({
  template: ''
})
class DummyComponent { }

describe('AlertseventsComponent', () => {
  let component: AlertseventsComponent;
  let fixture: ComponentFixture<AlertseventsComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [AlertseventsComponent, DummyComponent],
      imports: [
        RouterTestingModule.withRoutes([
          { path: 'login', component: DummyComponent }
        ]),
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
        SharedModule,
        HttpClientTestingModule,
        PrimengModule,
        MaterialdesignModule
      ],
      providers: [
        ControllerService,
        LogService,
        LogPublishersService,
        AlerttableService,
        MatIconRegistry,
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(AlertseventsComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
