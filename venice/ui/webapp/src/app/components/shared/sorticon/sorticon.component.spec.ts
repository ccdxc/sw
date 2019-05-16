import { HttpClientTestingModule } from '@angular/common/http/testing';
import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { RouterTestingModule } from '@angular/router/testing';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { Table } from 'primeng/table';
import { Observable } from 'rxjs';
import { SorticonComponent } from './sorticon.component';


class MockTable {
  tableService = {
    sortSource$: new Observable()
  };
}

describe('SorticonComponent', () => {
  let component: SorticonComponent;
  let fixture: ComponentFixture<SorticonComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [SorticonComponent],
      imports: [
        RouterTestingModule,
        NoopAnimationsModule,
        HttpClientTestingModule,
        MaterialdesignModule,
      ],
      providers: [
        {
          provide: Table,
          useClass: MockTable
        },
      ]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(SorticonComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
