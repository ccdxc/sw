/**-----
 Angular imports
 ------------------*/
 import { HttpClientTestingModule } from '@angular/common/http/testing';
 import { Component } from '@angular/core';
 import { async, ComponentFixture, TestBed } from '@angular/core/testing';
 import { FormsModule, ReactiveFormsModule } from '@angular/forms';
 import { MatIconRegistry } from '@angular/material';
 import { NoopAnimationsModule } from '@angular/platform-browser/animations';
 import { RouterTestingModule } from '@angular/router/testing';
 import { SharedModule } from '@app/components/shared/shared.module';
 /**-----
  Venice web-app imports
  ------------------*/
 import { ControllerService } from '@app/services/controller.service';
 import { LogPublishersService } from '@app/services/logging/log-publishers.service';
 import { LogService } from '@app/services/logging/log.service';
 import { MaterialdesignModule } from '@lib/materialdesign.module';
 import { PrimengModule } from '@lib/primeng.module';
 import { UIConfigsService } from '@app/services/uiconfigs.service';
 import { EventsService } from '@app/services/events.service';
 import { SearchService } from '@app/services/generated/search.service';
 import { MonitoringService } from '@app/services/generated/monitoring.service';
 import { MessageService } from 'primeng/primeng';
 import { Table } from 'primeng/table';

 import { WidgetsModule } from 'web-app-framework';
 import { NewtechsupportComponent } from './newtechsupport.component';

describe('NewtechsupportComponent', () => {
  let component: NewtechsupportComponent;
  let fixture: ComponentFixture<NewtechsupportComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ NewtechsupportComponent ],
      imports: [
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
        SharedModule,
        HttpClientTestingModule,
        PrimengModule,
        MaterialdesignModule,
        RouterTestingModule,
        WidgetsModule
      ],
      providers: [
        ControllerService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        UIConfigsService,
        EventsService,
        SearchService,
        MonitoringService,
        MessageService
      ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(NewtechsupportComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
