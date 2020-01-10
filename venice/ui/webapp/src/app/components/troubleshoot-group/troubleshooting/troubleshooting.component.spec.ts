import { async, ComponentFixture, TestBed } from '@angular/core/testing';

/**-----
 Angular imports
 ------------------*/
 import { HttpClientTestingModule } from '@angular/common/http/testing';
 import { HttpClientModule } from '@angular/common/http';
 import { Component } from '@angular/core';
 import { configureTestSuite } from 'ng-bullet';
 import { FormsModule, ReactiveFormsModule } from '@angular/forms';
 import { MatIconRegistry } from '@angular/material';
 import { NoopAnimationsModule } from '@angular/platform-browser/animations';
 import { RouterTestingModule } from '@angular/router/testing';
 import { SharedModule } from '@app/components/shared/shared.module';
 /**-----
  Venice web-app imports
  ------------------*/
 import { ControllerService } from '@app/services/controller.service';
 import { ConfirmationService } from 'primeng/primeng';
 import { LogPublishersService } from '@app/services/logging/log-publishers.service';
 import { LogService } from '@app/services/logging/log.service';
 import { MaterialdesignModule } from '@lib/materialdesign.module';
 import { PrimengModule } from '@lib/primeng.module';
 import { UIConfigsService } from '@app/services/uiconfigs.service';
 import { EventsService } from '@app/services/events.service';
 import { SearchService } from '@app/services/generated/search.service';
 import { MonitoringService } from '@app/services/generated/monitoring.service';
 import { MessageService } from '@app/services/message.service';
 import { WidgetsModule } from 'web-app-framework';



import { TroubleshootingComponent } from './troubleshooting.component';
import { NetworkgraphComponent } from './networkgraph/networkgraph.component';




describe('TroubleshootingComponent', () => {
  let component: TroubleshootingComponent;
  let fixture: ComponentFixture<TroubleshootingComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [ TroubleshootingComponent, NetworkgraphComponent ],
      imports: [
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
        SharedModule,
        HttpClientTestingModule,
        HttpClientModule,
        PrimengModule,
        MaterialdesignModule,
        RouterTestingModule,
        WidgetsModule
      ],
      providers: [
        ControllerService,
        UIConfigsService,
        ConfirmationService,
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
  });

  beforeEach(() => {
    fixture = TestBed.createComponent(TroubleshootingComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
