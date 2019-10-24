# Copyright (C) 2019 OpenIO SAS, as part of OpenIO SDS
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 3.0 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library.

import random

from oio.xcute.common.backend import XcuteBackend
from oio.xcute.common.exceptions import UnknownJobTypeException
from oio.common.green import datetime, time
from oio.xcute.jobs import JOB_TYPES


class XcuteManager(object):

    STATUS_WAITING = 'WAITING'
    STATUS_RUNNING = 'RUNNING'
    STATUS_PAUSED = 'PAUSED'
    STATUS_FINISHED = 'FINISHED'
    STATUS_FAILED = 'FAILED'

    def __init__(self, conf, logger):
        self.conf = conf
        self.backend = XcuteBackend(self.conf)
        self.logger = logger

    def create_job(self, job_type, job_conf=None):
        """
            Create a job (not started)
        """

        if job_type not in JOB_TYPES:
            raise UnknownJobTypeException()

        now = time.time()

        job_id = self._uuid()
        job_conf = job_conf or {}
        job_info = {
            'job_type': job_type,
            'ctime': now,
            'mtime': now,
            'status': self.STATUS_WAITING,
            'sent': 0,
            'all_sent': 0,
            'processed': 0,
            'errors': 0,
        }

        self.backend.create_job(job_id, job_conf, job_info)

        return dict(id=job_id)

    def get_orchestrator_jobs(self, orchestrator_id):
        """
            Get the list of jobs managed by a given orchestrator
        """

        return self.backend.list_orchestrator_jobs(orchestrator_id)

    def get_new_jobs(self, orchestrator_id):
        """
            Get waiting jobs until there's none left
        """

        return iter(lambda: self.backend.pop_job(orchestrator_id), None)

    def start_job(self, job_id, job_conf):
        """
            Mark a job as running
        """

        updates = {
            'status': self.STATUS_RUNNING,
            'mtime': time.time(),
        }
        self.backend.start_job(job_id, job_conf, updates)

    def pause_job(self, job_id):
        """
            Mark a job as paused
        """

        updates = {
            'status': self.STATUS_PAUSED,
            'mtime': time.time(),
        }
        self.backend.update_job_info(job_id, updates)

    def fail_job(self, job_id):
        """
            Mark a job as failed
        """

        updates = {
            'status': self.STATUS_FAILED,
            'mtime': time.time(),
        }
        self.backend.update_job_info(job_id, updates)

    def task_sent(self, job_id, task_id, total=None):
        """
            Update a job's sent tasks status
        """

        updates = {
            'mtime': time.time(),
        }
        if total is not None:
            updates['total'] = total
        self.backend.incr_sent(job_id, task_id, updates)

    def all_tasks_sent(self, job_id, nb_sent):
        """
            Mark a job as having all its tasks sent
        """

        updates = {
            'sent': nb_sent,
            'all_sent': 1,
            'mtime': time.time(),
        }
        self.backend.update_job_info(job_id, updates)

    def task_processed(self, orchestrator_id, job_id, task_id, task_ok):
        """
            Update a job's processed tasks status
        """

        updates = {
            'mtime': time.time(),
        }
        return self.backend.incr_processed(orchestrator_id, job_id,
                                           task_id, not task_ok, updates)

    def list_jobs(self, **kwargs):
        """
            Get all jobs with their information
        """

        return self.backend.list_jobs(**kwargs)

    def show_job(self, job_id):
        """
            Get one job and its information
        """

        return self.backend.get_job_info(job_id)

    def delete_job(self, job_id):
        """
            Delete a job
        """

        self.backend.delete_job(job_id)

    @staticmethod
    def _uuid():
        return datetime.utcnow().strftime('%Y%m%d%H%M%S%f') \
            + '-%011x' % random.randrange(16**10)