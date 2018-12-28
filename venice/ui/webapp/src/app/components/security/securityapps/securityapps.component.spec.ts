import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { SecurityappsComponent } from './securityapps.component';

import { RouterTestingModule } from '@angular/router/testing';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { SharedModule } from '@app/components/shared/shared.module';
import { ControllerService } from '@app/services/controller.service';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MatIconRegistry } from '@angular/material';
import { SecurityService } from '@app/services/generated/security.service';
import { PrimengModule } from '@app/lib/primeng.module';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { MessageService } from 'primeng/primeng';

describe('SecurityappsComponent', () => {
  let component: SecurityappsComponent;
  let fixture: ComponentFixture<SecurityappsComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ SecurityappsComponent ],
      imports: [
        RouterTestingModule,
        NoopAnimationsModule,
        HttpClientTestingModule,
        SharedModule,
        PrimengModule,
      ],
      providers: [
        ControllerService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        SecurityService,
        UIConfigsService,
        MessageService
      ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(SecurityappsComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
