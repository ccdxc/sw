import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { WidgetsModule } from 'web-app-framework';
import { PrimengModule } from '@app/lib/primeng.module';

import { SystemcapacitywidgetComponent } from './systemcapacity.component';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { SharedModule } from '@app/components/shared/shared.module';
import { ControllerService } from '@app/services/controller.service';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { MatIconRegistry } from '@angular/material';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { MessageService } from 'primeng/primeng';
import { RouterTestingModule } from '@angular/router/testing';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';


describe('SystemcapacitywidgetComponent', () => {
  let component: SystemcapacitywidgetComponent;
  let fixture: ComponentFixture<SystemcapacitywidgetComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [SystemcapacitywidgetComponent],
      imports: [
        WidgetsModule,
        PrimengModule,
        MaterialdesignModule,
        RouterTestingModule,
        HttpClientTestingModule,
        NoopAnimationsModule,
        SharedModule
      ],
      providers: [
        ControllerService,
        LogService,
        LogPublishersService,
        ClusterService,
        MatIconRegistry,
        UIConfigsService,
        MessageService,
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(SystemcapacitywidgetComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
